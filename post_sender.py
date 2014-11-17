import requests

payload1 = {"chord": 'g'}
payload2 = {"is_playing": 't'}
payload3 = {"chord": 'gm', "is_playing": 't'}
for payload in [payload1, payload2, payload3]:
    r = requests.post("http://127.0.0.1:8082/state", data=payload)
    print r.text
r = requests.get("http://127.0.0.1:8082/state") 
print r.text
