"""
    plot performance
"""

import os
from re import X
from pathlib import Path
import pandas as pd
from pytest import mark
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt
from labellines import labelLines
import itertools

def cycle_markers(iterable,n):
  for item in itertools.cycle(iterable):
    yield item

sns.set_theme()
markers = cycle_markers(('^','o','s'),1)

PATH = './'

fileNames = os.listdir(PATH)

fileNames = [file for file in fileNames if '.csv' in file]

for file in fileNames:
    df = pd.read_csv(PATH + file, usecols= [0,1,2])
    x = df.iloc[:,0].to_numpy()
    y = df.iloc[:,2].to_numpy()/df.iloc[:,1].to_numpy()
    #sort array after n
    perm = x.argsort()
    y = y[perm]
    x = x[perm]
    marker = next(markers)
    plt.plot(x, y, label = f"{Path(file).stem}", marker=marker)

plt.title("Belief Propagation [Processor, Flags ...]")
plt.xlabel('n')
plt.ylabel('Performance [F/C]')
#plt.legend()
plt.ylim(ymin=0,ymax=1)
plt.ylabel('flops/cycle')
#plt.grid()
ax = plt.gca()
ax.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax.grid(b=True, which='major', color='w', linewidth=1.0)
ax.grid(b=True, which='minor', color='w', linewidth=0.5)
labelLines(ax.get_lines(), yoffsets=0.1)
### Generate the plot
plt.savefig('./plots/performance_plot.png')  
plt.show() 
plt.close()