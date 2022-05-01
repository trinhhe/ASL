"""
    plot performance
"""

import os
from re import X
import pandas as pd
import matplotlib.pyplot as plt

PATH = './'

fileNames = os.listdir(PATH)

fileNames = [file for file in fileNames if '.csv' in file]

for file in fileNames:
    df = pd.read_csv(PATH + file, usecols= [0,1,2])
    x = df.iloc[:,0].to_numpy()
    y = df.iloc[:,2].to_numpy()/df.iloc[:,1].to_numpy()
    plt.plot(x, y, label = f"{file}")

plt.xlabel('n')
plt.ylabel('Performance [F/C]')
plt.grid()
plt.legend()
plt.ylim(ymin=0,ymax=2)
### Generate the plot
plt.savefig('./plots/performance_plot.png')  
plt.show() 
plt.close()