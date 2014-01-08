import json
import urllib2

data = {
        'type': 'test',
        'data': '1234'
        }


req = urllib2.Request('http://localhost:3000/test')
req.add_header('Content-Type', 'application/json')

num = 0
while (num < 10):
  response = urllib2.urlopen(req, json.dumps(data))
  num = num + 1

