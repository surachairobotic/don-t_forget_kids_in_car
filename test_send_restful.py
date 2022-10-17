# importing the requests library
import requests, json, base64

# defining the api-endpoint 
api_url = "http://127.0.0.1:8000/upload"
image_file = '/home/thinkpad/Pictures/Home-icon.png'

headers = {'Content-type': 'application/json', 'Accept': 'text/plain'}

data = open(image_file, 'rb').read()

with open(image_file, "rb") as f:
    im_bytes = f.read()        
im_b64 = base64.b64encode(im_bytes).decode("utf8")
#print(im_b64)

payload = json.dumps({"image": im_b64})

response = requests.post(api_url, data=payload, headers=headers)
try:
    data = response.json()     
    print(data)                
except requests.exceptions.RequestException:
    print(response.text)

exit()

# your API key here
API_KEY = "XXXXXXXXXXXXXXXXX"
  
# your source code here
source_code = '''
print("Hello, world!")
a = 1
b = 2
print(a + b)
'''
  
# data to be sent to api
data = {'api_dev_key':API_KEY,
        'api_option':'upload',
        'api_paste_code':source_code,
        'api_paste_format':'python'}

# sending post request and saving response as response object
res = requests.post(url=api_url, data=payload, headers=headers)

# extracting response text 
print(type(res))
print(res)

res = res.json()

print(type(res))
print(res)
