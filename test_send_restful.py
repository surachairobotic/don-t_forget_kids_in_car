# importing the requests library
import requests
  
# defining the api-endpoint 
API_ENDPOINT = "http://127.0.0.1:8000/upload"

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
res = requests.post(url = API_ENDPOINT, data = data)

# extracting response text 
print(type(res))
print(res)

res = res.json()

print(type(res))
print(res)