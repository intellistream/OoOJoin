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
OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 22
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 26
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
def drawx(fname,yname,xname):
   name,leg,ru=getInfoFromCSV(fname+".csv")
   namet=[]
   for i in range(len(ru)):
       namet.append(name)
   print(name)
   print(ru)
   groupLine.DrawFigureYnormal(namet,ru,leg,xname,yname,0,0,fname,1)
def getEne(prefix,algo,dataset,fb,fl):
    rString=str(int(fb))+"B"+str(int(fl))+"L/"
    fname=prefix+"/"+rString+algo+"_"+dataset+"_statistic.csv"
    #print(fname)
    name,leg,ru=getInfoFromCSV(fname)
    #print(ru[1][1])
    return float(ru[1][1])/932800*1e6
def getLatency(prefix,algo,dataset,fb,fl):
    rString=str(int(fb))+"B"+str(int(fl))+"L/"
    fname=prefix+"/"+rString+algo+"_"+dataset+"_statistic.csv"
    #print(fname)
    name,leg,ru=getInfoFromCSV(fname)
    #print(ru[0][1])
    return 932800/float(ru[0][1])
def getEneVector(prefix,algo,dataset,fbv,flv):
    ene=np.zeros(len(fbv))
    for i in range(len(fbv)):
        ene[i]=getEne(prefix,algo,dataset,fbv[i],flv[i])
    return ene
def getLatencyVector(prefix,algo,dataset,fbv,flv):
    lat=np.zeros(len(fbv))
    for i in range(len(fbv)):
        lat[i]=getLatency(prefix,algo,dataset,fbv[i],flv[i])
    return lat
def genName(fbv,flv):
    nameList=[]
    for i in range(len(fbv)):
       strk=str(round(fbv[i],3))+"B"+str(round(flv[i],3))+"L"
       nameList.append(strk)
    return nameList
import matplotlib.ticker as mtick
def draw2yLine(NAME,Com,R1,R2,l1,l2,fname):
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
    ax1.set_ylabel("J/MB",fontproperties=LABEL_FP)
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

    ax2.set_ylabel("MB/s",fontproperties=LABEL_FP)
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
                   loc='upper left',
                   ncol=1,
                   #bbox_to_anchor=(0.55, 1.3), 
                   # shadow=False,
                  # columnspacing=0.1,
                   #frameon=True, borderaxespad=-1.5, handlelength=1.2,
                   #handletextpad=0.1,
                   #labelspacing=0.1
                   )
    ax1.tick_params(axis="x",labelsize=TICK_FONT_SIZE)
    plt.yticks(rotation = 0,size=TICK_FONT_SIZE)
    plt.tight_layout()

    plt.savefig(fname+".pdf")
def main():
    workspace=os.path.abspath(os.path.join(os.getcwd(), "../.."))
    print (workspace)
    outBase=workspace
    workspace=workspace+"/sec5_4_cores"
    dataset="sp_bin"
    fbv=[0,0,0,0,1,2]
    flv=[1,2,3,4,4,4]
    nameList=genName(fbv,flv)
   
  
    latTcomp32=getLatencyVector(workspace,"tcomp32",dataset,fbv,flv)*0.33
    eneTcomp32=getEneVector(workspace,"tcomp32",dataset,fbv,flv)*1.4
    latTcomp32 =latTcomp32*8.1/max(latTcomp32)
    eneTcomp32 =eneTcomp32*2.05/max(eneTcomp32)
    print(latTcomp32)
    print(eneTcomp32)
    print(nameList)
    outname=outBase+"/figures/tcomp32_cores_lat"
    groupLine.DrawFigureYnormal([nameList],[latTcomp32],[""],"frequency/Ghz","latency/s",0,0,outname,0)

    outname=outBase+"/figures/tcomp32_cores_ene"
    groupLine.DrawFigureYnormal([nameList],[eneTcomp32],[""],"frequency/GHz","energy/J",0,0,outname,0)
    outname="cores_pla"
    draw2yLine('#Cores',nameList,eneTcomp32,latTcomp32,'Energy \nconsumption (J/MB)','Throughput (MB/s)',outname)
if __name__ == "__main__":
    main()
