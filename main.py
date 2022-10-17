import shutil, json, base64, threading, copy
import numpy as np
from PIL import Image 

from fastapi import FastAPI, Request, File, UploadFile
from fastapi.encoders import jsonable_encoder
import sys
sys.setrecursionlimit(15000)

app = FastAPI()
cnt=0
all_data = []
w = 0
h = 0
bStart = False

@app.get("/")
def read_root():
    return {"Hello": "World"}

'''
@app.post("/upload")
async def upload(image: UploadFile = File(...)):
    global cnt
    print(cnt)
    cnt+=1
    
    print(type(image))
    print(image)
    
    with open("destination.png", "wb") as buffer:
        shutil.copyfileobj(image.file, buffer)
    
    return {"filename": image.filename}
    return {"upload": "image"}

'''

@app.post("/upload")
async def upload(req: Request): # var_name: var_type
    global cnt
    #content = '%s %s' % (req.method, req.url.path)
    #print(content)
    #print('%s, %s, %s' % (req.url.path, req.url.port, req.url.scheme))
    req_info = await req.json()
    str_image = req_info['image']
    #print(type(str_image))
    #binary_image = req_info['image'].encode('ascii')
    #print(type(binary_image))
    
    print(str_image)
    
    fh = open("img_{}.png".format(cnt), "wb")
    fh.write(base64.b64decode(str_image))
    fh.close()
    cnt+=1

    return json.dumps({"upload": "image"})

@app.post("/upload_raw")
async def upload(req: Request): # var_name: var_type
    global all_data, w, h, bStart
    #json_compat = jsonable_encoder(req)
    # print(json_compat)    
    # content = '%s %s' % (req.method, req.url.path)
    # print(content)
    # print('%s, %s, %s' % (req.url.path, req.url.port, req.url.scheme))
    # print(req)
    req_info = await req.json()
    #print(req_info)
    
    str_image = req_info['image']
    #print(req_info['width'])
    #print(req_info['height'])
    #print(req_info['len'])
    id = int(req_info['id'])
    print(id)
    if id == 0:
        all_data = []
        w = int(req_info['width'])
        h = int(req_info['height'])
        bStart = True
    #print(type(str_image))
    #print(len(str_image))
    data = filter(None, str_image.split(','))
    #print(data)
    iData = [int(x) for x in data]
    all_data.append(iData)

    if id == 19 and bStart:
        threadMerge = threading.Thread(target=funcDataProcess)
        threadMerge.start()
    #print(iData)
    '''

    str_img = str_image.split(",")
    img = [int(x) for x in str_img]
    print(img)
    print(len(img))
    #binary_image = req_info['image'].encode('ascii')
    #print(type(binary_image))
    '''
    
    '''
    msg = ""
    c=0
    for x in str_image:
        msg += "{},".format(x)
        c+=1
    print(msg)
    print(c)
    '''

def funcDataProcess():
    global all_data, w, h, cnt
    print('funcDataProcess')
    local_data = copy.deepcopy(all_data)

    data = []
    for d in local_data:
        data += d
    
    print([w, h])

    fp = open(r'debug.txt', 'w')
    fp.write("data:\n");
    c=0
    for x in data:
        fp.write("{},".format(x))
        c+=1
        if c==w:
            c=0
            fp.write('\n')
    fp.close()

    arr = np.asarray(data, dtype=np.uint8)
    img = Image.fromarray(arr.reshape(w,h), 'L')
    img.save('gray_{}.jpg'.format(cnt))
    cnt+=1
    
    #print(data)
    print('end')