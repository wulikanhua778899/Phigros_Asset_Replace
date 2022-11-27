import os
import json
import csv
import glob
import shutil

sourcePath = input("请输入Asset Studio导出位置（绝对路径）:")

againInput = True
while(againInput):
    print('1 > 重命名并导出官方谱面')
    print('2 > 不导出谱面 仅输出信息')
    parseMode = int(input('Please choose (1 Or 2):'))
    if(parseMode == 1):
        if(not os.path.exists('Output')):
            os.mkdir('Output')
        outputFlag = True
        againInput = False
    if(parseMode == 2):
        outputFlag = False
        againInput = False

Name2BundleList = []
resultList = []
noProcessList = []  # 未处理对象列表
with open('result.csv', 'r', newline='', encoding='utf-8') as f:
    rows = 0  # 计算csv行数
    while f.readline() != '':
        rows = rows + 1

    f.seek(0, os.SEEK_SET)
    csv_data = csv.DictReader(f)
    for count in range(rows - 1):
        item = csv_data.__next__()
        resultItem = {}
        resultItem['name'] = item['Name']
        resultItem['difficulty'] = item['Difficulty']
        resultItem['numOfNotes'] = item['Notes']
        resultItem['bpm'] = item['BPM']
        resultList.append(resultItem)

# sourcePath = 'F:\\Phigros\\新建文件夹'

# sourceName[:39]
for sourceName in glob.glob('*', root_dir=sourcePath):
    print(sourceName[:39])
    if('bundle' not in sourceName):
        break
    for cabName in glob.glob('*', root_dir=sourcePath + '\\' + sourceName):
        for fileName in glob.glob('*', root_dir=sourcePath + '\\' + sourceName + '\\' + cabName):
            print(fileName)
            with open(sourcePath + '\\' + sourceName + '\\' + cabName + '\\' + fileName, mode="r") as f:
                data = json.loads(f.read())
                numOfNotes = data.get("numOfNotes")  # 调取物量数据
                bpm = data.get('judgeLineList')[0].get('bpm')  # 调取BPM信息

            searchflag = False
            for resultItem in resultList:
                if str(numOfNotes) == resultItem['numOfNotes'] and str(bpm) == resultItem['bpm'] and fileName[6:8].upper() == resultItem['difficulty']:
                    chartName = resultItem['name'] + ' - ' + resultItem['difficulty']
                    Name2BundleList.append({'Name': chartName, 'Bundle': sourceName[:39]})

                    if(outputFlag):
                        shutil.copy(sourcePath + '\\' + sourceName + '\\' + cabName + '\\' + fileName, 'Output\\' + chartName + '.json')

                    resultList.remove(resultItem)
                    searchflag = True
                    break
            if not searchflag:
                noProcessList.append(str(sourceName[:39] + ' numOfNotes:' +
                                         str(numOfNotes) + ' - ' + 'bpm:' + str(bpm) + ' 未查找到数据'))

with open('Name2Bundle.csv', 'w', newline='', encoding='utf-8') as f:
    csv_data = csv.DictWriter(f, ['Name', 'Bundle'])
    csv_data.writeheader()
    for item in Name2BundleList:
        csv_data.writerow(item)
    f.close()

# 执行结果输出
with open("Output_Info.txt", mode="w") as f:
    def Output(massage):
        print(massage)
        print(massage, file=f)

    Output("未处理对象：")
    for item in noProcessList:
        Output(item)

    Output('')  # 换行

    Output("数据表未匹配：")
    for item in resultList:
        Output(item)
