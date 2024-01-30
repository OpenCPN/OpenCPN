from websocket import *

import random
import string
import ssl

def randomString(stringLength=10):
    """Generate a random string of fixed length """
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(stringLength))

st = randomString(32768)

with open('generated_file', 'w') as f:
    f.write(st)

ws = create_connection("wss://echo.websocket.org/",
    sslopt={"cert_reqs": ssl.CERT_NONE})

print("Sending")

frame = ABNF.create_frame(st, ABNF.OPCODE_TEXT, 0)
ws.send_frame(frame)
cont_frame = ABNF.create_frame(st, ABNF.OPCODE_CONT, 0)
ws.send_frame(cont_frame)
cont_frame = ABNF.create_frame(st, ABNF.OPCODE_CONT, 1)
ws.send_frame(cont_frame)

print("Sent")
print("Receiving...")
result =  ws.recv()
if st+st+st  == result:
    print("Received ")
else:
    print("Error")
ws.close()
