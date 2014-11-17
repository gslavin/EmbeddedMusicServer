import requests

payload = {"is_playing": "1", "chord": 'g'}
r = requests.post("http://127.0.0.1:8082/state", data=payload)
print r.text
r = requests.get("http://127.0.0.1:8082/state") 
print r.text
