import requests

payload1 = {"chord": 'g'}
payload2 = {"control": 'start'}
payload3 = {"chord": 'gm', "control": 'start'}
for payload in [payload1, payload2, payload3]:
    r = requests.post("http://127.0.0.1:8082/state", data=payload)
    print r.text
r = requests.get("http://127.0.0.1:8082/state") 
print r.text
