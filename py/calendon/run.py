import queue
import subprocess
import threading
from typing import IO, List


def _read_stream(stream: IO, queued_lines: queue.Queue):
    """Reads a stream into a queue."""
    for line in stream:
        try:
            queued_lines.put(line.decode().strip())
        except UnicodeDecodeError:
            queued_lines.put(line.strip())


def run_program(command_line: List[str], **kwargs):
    """Runs another process while streaming its stdout and stderr."""
    print(f'Running: {" ".join(command_line)} {kwargs}')
    process = subprocess.Popen(command_line,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               **kwargs)

    out_queue: queue.Queue = queue.Queue()
    err_queue: queue.Queue = queue.Queue()

    out_thread = threading.Thread(target=_read_stream, args=(process.stdout, out_queue))
    err_thread = threading.Thread(target=_read_stream, args=(process.stderr, err_queue))

    out_thread.start()
    err_thread.start()

    while out_thread.is_alive() or err_thread.is_alive() or not out_queue.empty() or not err_queue.empty():
        try:
            line = out_queue.get_nowait()
            print(line)
        except queue.Empty:
            pass

        try:
            line = err_queue.get_nowait()
            print(line)
        except queue.Empty:
            pass

    out_thread.join()
    err_thread.join()

    return process.wait()
