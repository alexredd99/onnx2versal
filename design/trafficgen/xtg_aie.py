from typing import List, Callable, Any, Tuple
import re

import numpy as np
import multiprocessing as mp
import struct
import logging

from xilinx_xtlm import ipc_axis_master_util, ipc_axis_slave_util, xtlm_ipc

logging.basicConfig(format="%(asctime)s: %(message)s", level=logging.INFO, datefmt="%H:%M:%S")


# See https://docs.python.org/3/library/struct.html#format-characters
# Expects [real, imag, real, imag, ...], each in cint16
# Combines pairs into 2's complement data
def cint16_tobytes(data: List[int]):
  if len(data) % 2 != 0:
    return ValueError(f"Expected even number of values, found line {data}.")
  data = np.real(data)
  real = ((int('0xFFFF',16) + int('0x1',16)) + data[::2]) & 0xFFFF
  imag = ((int('0xFFFF',16) + int('0x1',16)) + data[1::2]) & 0xFFFF
  return real | imag << 16

def cint16_fstr_send(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//4)+"I" 

def cint16_fstr_recv(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//4)+"i" 

def uint8_tobytes(data: List[int]):
  return np.real(data).astype(np.uint8)

def uint8_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//1)+"B" 

def int8_tobytes(data: List[int]):
  return np.real(data).astype(np.int8)

def int8_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//1)+"b" 

def int16_tobytes(data: List[int]):
  return np.real(data).astype(np.int16).tolist()

def int16_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//2)+"h"

def int32_tobytes(data: List[int]):
  return np.real(data).astype(np.int32).tolist()

def int32_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//4)+"i"

def float32_tobytes(data: List[int]):
  return np.real(data).astype(np.float32).tolist()

def float32_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//4)+"f"

def get_format_to_bytes_callable(dtype: str):
  if dtype == "int32":
    return int32_tobytes
  elif dtype == "int16":
    return int16_tobytes
  elif dtype == "int8":
    return int8_tobytes
  elif dtype == "uint8":
    return uint8_tobytes
  elif dtype == "cint16":
    return cint16_tobytes
  elif dtype == "float32":
    return float32_tobytes
  else:
    raise NotImplementedError(f"{dtype} formatting not supported.")

def get_format_string_callable_send(dtype: str):
  if dtype == "int32":
    return int32_fstr
  elif dtype == "int16":
    return int16_fstr
  elif dtype == "int8":
    return int8_fstr
  elif dtype == "uint8":
    return uint8_fstr
  elif dtype == "cint16":
    return cint16_fstr_send
  elif dtype == "float32":
    return float32_fstr
  else:
    raise NotImplementedError(f"{dtype} format string not supported.")

def get_format_string_callable_recv(dtype: str):
  if dtype == "int32":
    return int32_fstr
  elif dtype == "int16":
    return int16_fstr
  elif dtype == "int8":
    return int8_fstr
  elif dtype == "uint8":
    return uint8_fstr
  elif dtype == "cint16":
    return cint16_fstr_recv
  elif dtype == "float32":
    return float32_fstr
  else:
    raise NotImplementedError(f"{dtype} format string not supported.")


class ExternalTraffic:
  
  def __init__(self,
               master_list: List[Tuple[str, str, int, str]],
               slave_list: List[Tuple[str, str, str, int]]):
    
    self.ipc_masters = []
    for ipc_name, file_path, bitwidth, dtype in master_list:
      self.ipc_masters.append(
        (ipc_axis_master_util(ipc_name), ipc_name, file_path, bitwidth, dtype))
      logging.info(f"Creating ipc_axis_master_util for {ipc_name}...")

    self.ipc_slaves = []
    for ipc_name, file_path, bitwidth, dtype, recv_len in slave_list:
      parent, child = mp.Pipe()
      self.ipc_slaves.append(
        (ipc_axis_slave_util(ipc_name), parent, child, ipc_name, file_path, bitwidth, dtype, recv_len))
      logging.info(f"Creating ipc_axis_slave_util for {ipc_name}...")

  def send_to_aie(self,
                  ipc_name: str,
                  file_path: str,
                  bitwidth: int, 
                  dtype: str,
                  transport: Callable[[Any], None] #xtlm_ipc_axis_pb2.axi_stream_packet
                  ):
    """Sending data to AIE from memory"""
    format_to_bytes = get_format_to_bytes_callable(dtype)
    get_format_string = get_format_string_callable_send(dtype)

    with open(file_path) as f:
      L = f.readlines()
      logging.info(f"[{ipc_name}]: Sending {len(L)} lines of {dtype} data...")
      
      for i, line in enumerate(L):
        values = line.split()
        packet = format_to_bytes(values)

        payload = xtlm_ipc.axi_stream_packet()
        payload.data_length = bitwidth // 8 # in bytes
        payload.tlast = True

        format_string = get_format_string(payload.data_length)
        try:
          payload.data = bytes(bytearray(struct.pack(format_string, *tuple(packet))))
          transport(payload)
        except Exception as e:
          logging.info(f"[{ipc_name}]: {e}")
          logging.info(f"[{ipc_name}]: fmtstr {format_string}; packet {packet};" + \
                       f" file_path {file_path}; line {line}; i {i}")

  def recv_fr_aie(self,
                  ipc_name: str,
                  dtype: str,
                  ipc_slave: ipc_axis_slave_util,
                  child: Any, # mp.connection.Connection
                  recv_len: int): 
    """Receiving data from AIE to memory"""
    get_format_string = get_format_string_callable_recv(dtype)
    rxData = []
    lines = 0

    while len(rxData) < recv_len:
      payload = ipc_slave.sample_transaction()
      formatString = get_format_string(len(payload.data))
      rxData += struct.unpack(formatString, payload.data)
      logging.info(f"[{ipc_name}]: read lines #{lines}")
      
      lines += 1
      
    child.send(rxData)

  def write(self,
            bitwidth: int,
            dtype: str,
            file_path: str,
            data: List[str]):
    repeat_count = bitwidth // int(re.findall(r'\d+', dtype)[0])
    if "c" in dtype:
      repeat_count // 2
    
    if dtype == "cint16":
      raw = data
      data = list()
      for d in raw:
        data += [np.int16(d & 0xFFFF), d >> 16]
    
    if len(data) % repeat_count != 0:
      data += [0] * ((repeat_count - len(data) % repeat_count) % repeat_count)

    tmp = ""
    with open(file_path, 'w') as f:
      for i, d in enumerate(data):
        tmp += f"{d} "
        if i % repeat_count == repeat_count - 1:
          f.write(f"{tmp}\n")
          tmp = ""

  def run(self):
    logging.info("Begin run...")
    master_tasks = []
    
    for ipc_master, ipc_name, file_path, bitwidth, dtype in self.ipc_masters:
      t = mp.Process(target=self.send_to_aie, 
                     args=(ipc_name, file_path, bitwidth, dtype, ipc_master.b_transport))
      t.start()
      master_tasks.append((t, ipc_name))
      logging.info(f"Running master {ipc_name}")

    slave_tasks = []
    for ipc_slave, parent, child, ipc_name, file_path, bitwidth, dtype, recv_len in self.ipc_slaves:
      t = mp.Process(target=self.recv_fr_aie, 
                     args=(ipc_name, dtype, ipc_slave, child, recv_len))
      t.start()
      slave_tasks.append((t, parent, ipc_name, file_path, bitwidth, dtype))
      logging.info(f"Running slave {ipc_name}")

    for slave_task, parent, ipc_name, file_path, bitwidth, dtype in slave_tasks:
      data = parent.recv()
      slave_task.join()
      self.write(bitwidth, dtype, file_path, data)
      logging.info(f"Slave {ipc_name} finished. Written to {file_path}")
    
    for master_task, ipc_name in master_tasks:
      master_task.join()
      logging.info(f"Master {ipc_name} finished.")
