#!/usr/bin/env python3
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
import sys

show = True
if len(sys.argv) > 1 and sys.argv[1] == "-n":
    show = False

def cycle_markers(iterable,n):
  for item in itertools.cycle(iterable):
    yield item

sns.set_theme()
markers = cycle_markers(('^','o','s'),1)

PATH = os.path.join(os.path.dirname(__file__), '../measurements/')

fileNames = os.listdir(PATH)

fileNames = [file for file in fileNames if '.csv' in file]

for file in fileNames:
    df = pd.read_csv(PATH + file, usecols= [0,1,2], index_col=False)
    x = df.iloc[:,0].to_numpy()
    y = df.iloc[:,2].to_numpy()/df.iloc[:,1].to_numpy()
    #sort array after n
    perm = x.argsort()
    y = y[perm]
    x = x[perm]
    marker = next(markers)
    pretty_name = Path(file).stem.replace("@", " ").replace("__", "=")
    plt.plot(x, y, label = pretty_name, marker=marker)

plt.title("Belief Propagation [Processor, Flags ...]")
plt.xlabel('n')
plt.ylabel('Performance [F/C]')
#plt.legend()
plt.ylim(ymin=0,ymax=0.6)
plt.ylabel('flops/cycle')
#plt.grid()
ax = plt.gca()
ax.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax.grid(b=True, which='major', color='w', linewidth=1.0)
ax.grid(b=True, which='minor', color='w', linewidth=0.5)
labelLines(ax.get_lines(), 
            yoffsets=0.022, 
            fontsize=10, 
            align=False, 
            outline_color=None, 
            outline_width=0)
### Generate the plot
plt.savefig('./plots/performance_plot.png')
plt.savefig('./plots/performance_plot.pdf')
if show:
    plt.show()
plt.close()
