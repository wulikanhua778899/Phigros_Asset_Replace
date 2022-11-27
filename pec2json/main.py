# pip install pyexecjs
import subprocess
from functools import partial #用来固定某个参数的固定值
subprocess.Popen=partial(subprocess.Popen,encoding='utf-8')

import execjs

parseCall = ''
againInput = True
while(againInput):
    print('1 > parsePEC')
    print('2 > parseRPE')
    parseMode = int(input('Please choose parseMode(1 Or 2):'))
    if(parseMode == 1):
        parseCall = 'parse'
        againInput = False
    if(parseMode == 2):
        parseCall = 'parseRPE'
        againInput = False

filename = input("Please enter json filename:")

node = execjs.get()
with open('pec2json.js',mode='r',encoding='utf-8') as jsCodeFile:
    jsCode = jsCodeFile.read()
with open(filename,mode='r',encoding='utf-8') as dataFile:
    data = dataFile.read()

result = node.compile(jsCode).call(parseCall,data,filename)

with open('Output.txt',mode='w') as jsonFile:
    jsonFile.write(result['data'])
with open('message.txt',mode='w') as messagesFile:
    for item in result['messages']:
        messagesFile.write(item)
        print(item)

print('Success')