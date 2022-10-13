import shutil, json, base64
from fastapi import FastAPI, Request, File, UploadFile

app = FastAPI()
cnt=0

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
    
    fh = open("img_{}.png".format(cnt), "wb")
    fh.write(base64.b64decode(str_image))
    fh.close()
    cnt+=1

    return json.dumps({"upload": "image"})

