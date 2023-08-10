"""
Most fusion is done when parsing ONNX into AIEngine operators. 
The following is fused at ONNX level so quantized ONNX will have them fused as well.
This is to prevent extra dequantize/quantize operations.
  - Matmul + BatchNormalization
"""
from typing import Mapping
import argparse

import numpy as np
import onnx
from onnx import numpy_helper
from onnx.helper import make_model, make_node, make_graph
from onnx.checker import check_model


class Fuser:
  
  def __init__(self,
               onnx_path: str,
               output_onnx_path: str):
    self.output_onnx_path = output_onnx_path

    self.model = onnx.load(onnx_path)
    self.nodes = self.model.graph.node

    self.initializers: Mapping[str, np.ndarray] = {
      init.name: init for init in self.model.graph.initializer}
  
  def get_optype(self, i: int):
    if i >= len(self.nodes):
      return ""
    return self.nodes[i].op_type
  
  def fuse(self):
    nodes = []
    initializers = dict(self.initializers)

    i = 0
    while i < len(self.nodes):
      node = self.nodes[i]
      if node.op_type == "MatMul" and self.get_optype(i+1) == "BatchNormalization":

        mm = node
        mm_input_name, mm_weight_name = mm.input

        bn = self.nodes[i+1]
        eps = bn.attribute[0].f
        bn_input_name, bn_weight_name, bn_bias_name, bn_mean_name, bn_var_name = bn.input

        mm_weight = numpy_helper.to_array(self.initializers[mm_weight_name])
        bn_weight = numpy_helper.to_array(self.initializers[bn_weight_name])
        bn_bias = numpy_helper.to_array(self.initializers[bn_bias_name])
        bn_mean = numpy_helper.to_array(self.initializers[bn_mean_name])
        bn_var = numpy_helper.to_array(self.initializers[bn_var_name])

        gemm_weight = mm_weight / np.sqrt(bn_var + eps) * bn_weight
        gemm_bias = -bn_mean / np.sqrt(bn_var + eps) * bn_weight + bn_bias
        initializers[bn_weight_name] = numpy_helper.from_array(gemm_weight, name=bn_weight_name)
        initializers[bn_bias_name] = numpy_helper.from_array(gemm_bias, name=bn_bias_name)

        del initializers[mm_weight_name]
        del initializers[bn_mean_name]
        del initializers[bn_var_name]
        
        gemm = make_node("Gemm", 
                        [mm_input_name, bn_weight_name, bn_bias_name], 
                        bn.output)
        nodes.append(gemm)

        i += 1 # skip BatchNormalization
      
      elif node.op_type == "MatMul" and i == len(self.nodes) - 1:
        mm = node
        mm_input_name, mm_weight_name = mm.input
        mm_weight = numpy_helper.to_array(self.initializers[mm_weight_name])
        
        gemm_bias = np.zeros((mm_weight.shape[1]), dtype=np.float32)
        initializers[mm_weight_name + "_bias"] = numpy_helper.from_array(gemm_bias, name=mm_weight_name + "_bias")

        gemm = make_node("Gemm",
                         [mm_input_name, mm_weight_name, mm_weight_name+"_bias"],
                         mm.output)
        nodes.append(gemm)

      else:
        nodes.append(node)
      
      i += 1
    
    new_graph = make_graph(
      nodes,
      "my_graph",
      self.model.graph.input,
      self.model.graph.output,
      initializers.values()
    )
    
    new_model = make_model(new_graph)

    check_model(new_model)

    onnx.save(new_model, self.output_onnx_path)


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Generate new ONNX model with fused ops from ONNX model.')
  parser.add_argument("input_onnx",  nargs=1, help="required path to input onnx file")
  parser.add_argument("output_onnx", nargs=1, help="required path to output onnx file with fused ops")
  args = parser.parse_args()

  args.input_onnx = args.input_onnx[0]
  args.output_onnx = args.output_onnx[0]
  
  fuser = Fuser(args.input_onnx, args.output_onnx)
  fuser.fuse()
