from flask import Flask
from flask import request
import pymysql
import json

app = Flask("live-server")



#@app.route("/reg",method = ['POST','GET'])
@app.route("/reg", methods=['POST', 'GET'])
def reg():
    # 1.get the data from SERVER
    print('jjjj')
    json = request.json
    print(json)
    # 2.write it in db
    # to get the data by pymysql

    # 3.reback the data to SERVER
    # return
    return 'hello world'
app.run(host = '0.0.0.0',port = 10099)