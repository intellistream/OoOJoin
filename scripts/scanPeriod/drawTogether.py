#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar as groupBar
import groupBar2 as groupBar2
import groupLine as groupLine
from autoParase import *
import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator,LinearLocator
import os
import pandas as pd
import sys
OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 22
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['*', '|', 'v', "^", "", "h", "<", ">", "+", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (["////", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42
def getInfoFromCSV(a):
    #column legengd, row name
    with open(a, 'r') as f:
        reader = csv.reader(f)
        #reader = [each for each in csv.DictReader(f, delimiter=',')]
        result = list(reader)
        rows=len(result)
        #print('rows=',rows)
        firstRow = result[0]
        #print(firstRow)
        index=0
        #define what may attract our interest
        idxCpu=0
        idxName=0
        idxEng=0
        idxLat=0
        idxVio=0
        idxOve=0
        xt=[]
        yt=[]
        idxList=[]
        nameList=[]
        legendList=[]
        for i in firstRow:
            #print(i)
            if(i!='name'):
               idxt=index
               idxList.append(idxt)
            index=index+1
        idxName=0
        #get names
        for k in range(len(idxList)):
            namet=(result[0][idxList[k]])
            nameList.append(namet)
        #get legend
        for k in range(1,rows):
            legt=result[k][0]
            legendList.append(legt)
        #get results
        ruAll=[]
        ruRow=[]
        for k in range(1,rows):
            ruRow=[]
            for j in range(len(idxList)):
                rut=(result[k][idxList[j]])
                ruRow.append(rut)
            ruAll.append(ruRow)
        return nameList,legendList,ruAll

def editConfig(src,dest,key,value):
    df = pd.read_csv(src, header=None)
    rowIdx=0
    idxt=0
    for cell in df[0]:
        #print(cell)
        if(cell==key):
            rowIdx=idxt
            break
        idxt=idxt+1
    df[1][rowIdx]=str(value)
    df.to_csv(dest,index=False,header=False)
def readConfig(src,key):
    df = pd.read_csv(src, header=None)
    rowIdx=0
    idxt=0
    for cell in df[0]:
        #print(cell)
        if(cell==key):
            rowIdx=idxt
            break
        idxt=idxt+1
    return df[1][rowIdx]
   
def runPeriod(exePath,period,resultPath):
    #resultFolder="periodTests"
    configFname="config_period_"+str(period)+".csv"
    configTemplate="config.csv"
    #clear old files
    os.system("cd "+exePath+"&& rm *.csv")
    #prepare new file
    editConfig(configTemplate,exePath+configFname,"watermarkPeriod",period)
    #run 
    os.system("cd "+exePath+"&& ./benchmark "+configFname)
    #copy result
    os.system("rm -rf "+resultPath+"/"+str(period))
    os.system("mkdir "+resultPath+"/"+str(period))
    os.system("cd "+exePath+"&& cp *.csv "+resultPath+"/"+str(period))
def runPeriodVector(exePath,periodVec,resultPath):
    for i in periodVec:
        runPeriod(exePath,i,resultPath)
def readResultPeriod(period,resultPath):
    resultFname=resultPath+"/"+str(period)+"/default_general.csv"
    avgLat=readConfig(resultFname,"AvgLatency")
    lat95=readConfig(resultFname,"95%Latency")
    thr=readConfig(resultFname,"Throughput")
    err=readConfig(resultFname,"Error")
    return avgLat,lat95,thr,err
def readResultVectorPeriod(periodVec,resultPath):
    avgLatVec=[]
    lat95Vec=[]
    thrVec=[]
    errVec=[]
    compVec=[]
    for i in periodVec:
        avgLat,lat95,thr,err=readResultPeriod(i,resultPath)
        avgLatVec.append(float(avgLat)/1000.0)
        lat95Vec.append(float(lat95)/1000.0)
        thrVec.append(float(thr)/1000.0)
        errVec.append(abs(float(err)))
        compVec.append(1-abs(float(err)))
    return avgLatVec,lat95Vec,thrVec,errVec,compVec
import matplotlib.ticker as mtick
def draw2yLine(NAME,Com,R1,R2,l1,l2,m1,m2,fname):
    fig, ax1 = plt.subplots(figsize=(10,6.4)) 
    lines= [None] *2
    #ax1.set_ylim(0, 1)
    print(Com)
    print(R1)
    lines[0],=ax1.plot(Com, R1,   color=LINE_COLORS[0], \
                                linewidth=LINE_WIDTH, marker=MARKERS[0], \
                                markersize=MARKER_SIZE
                                # 
            )

    #plt.show()
    ax1.set_ylabel(m1,fontproperties=LABEL_FP)
    ax1.set_xlabel(NAME,fontproperties=LABEL_FP)
    #ax1.set_xticklabels(ax1.get_xticklabels())  # 设置共用的x轴
    plt.xticks(rotation = 0,size=TICK_FONT_SIZE)
    plt.yticks(rotation = 0,size=TICK_FONT_SIZE)
    ax2 = ax1.twinx()
    
    #ax2.set_ylabel('latency/us')
    #ax2.set_ylim(0, 0.5)
    lines[1],=ax2.plot(Com, R2, color=LINE_COLORS[1], \
                                linewidth=LINE_WIDTH, marker=MARKERS[1], \
                                markersize=MARKER_SIZE)

    ax2.set_ylabel(m2,fontproperties=LABEL_FP)
    #ax2.vlines(192000, min(R2)-1, max(R2)+1, colors = "GREEN", linestyles = "dashed",label='total L1 size')
    # plt.grid(axis='y', color='gray')

    #style = dict(size=10, color='black')
    #ax2.hlines(tset, 0, x2_list[len(x2_list)-1]+width, colors = "r", linestyles = "dashed",label="tset") 
    #ax2.text(4, tset, "$T_{set}$="+str(tset)+"us", ha='right', **style)
    

    #plt.xlabel('batch', fontproperties=LABEL_FP)
    
    #plt.xscale('log')
    # figure.xaxis.set_major_locator(LinearLocator(5))
    ax1.yaxis.set_major_locator(LinearLocator(5))
    ax2.yaxis.set_major_locator(LinearLocator(5))
    ax1.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.1f'))
    ax2.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.1f'))
    ax1.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.1f'))
    ax2.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.1f'))
    plt.legend(lines,
                   [l1,l2],
                   prop=LEGEND_FP,
                   loc='upper center',
                   ncol=1,
                   bbox_to_anchor=(0.55, 1.3
                   ), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=-1.5, handlelength=1.2,
                   handletextpad=0.1,
                   labelspacing=0.1)
    plt.yticks(rotation = 0,size=TICK_FONT_SIZE)
    plt.tight_layout()

    plt.savefig(fname+".pdf")
def main():
    exeSpace=os.path.abspath(os.path.join(os.getcwd(), "../.."))+"/"
    resultPath=os.path.abspath(os.path.join(os.getcwd(), "../.."))+"/results/periodTest/"
    figPath=os.path.abspath(os.path.join(os.getcwd(), "../.."))+"/figures/"
    configTemplate=exeSpace+"config.csv"
    periodVec=[50,80,100,110,120,130,140,150]
    periodVecDisp=np.array(periodVec)
    periodVecDisp=periodVecDisp*40/1000
    print(configTemplate)
    #run
    if(len(sys.argv)<2):
        os.system("rm -rf "+resultPath)
        os.system("mkdir "+resultPath)
        runPeriodVector(exeSpace,periodVec,resultPath)
    avgLatVec,lat95Vec,thrVec,errVec,compVec=readResultVectorPeriod(periodVec,resultPath)
    os.system("mkdir "+figPath)
    draw2yLine("watermark time (ms)",periodVecDisp,lat95Vec,errVec,"95% Latency (ms)","Error","ms","",figPath+"lat_err")
    draw2yLine("watermark time (ms)",periodVecDisp,thrVec,errVec,"Throughput (KTp/s)","Error","KTp/s","",figPath+"lat_thr")
    draw2yLine("watermark time (ms)",periodVecDisp,lat95Vec,compVec,"95% Latency (ms)","Completeness","ms","",figPath+"lat_comp")
    print(errVec)
    #readResultPeriod(50,resultPath)
if __name__ == "__main__":
    main()
