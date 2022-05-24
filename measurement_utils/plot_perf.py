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
OUT = os.path.join(os.path.dirname(__file__), "plots/")

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
    if not x.size:
        # the corresponding run has probably crashed, if we don't ignore it, we run into problems later
        continue
    marker = next(markers)
    pretty_name = Path(file).stem.replace("@", " ").replace("__", "=")
    plt.plot(x, y, label = pretty_name, marker=marker)

plt.title("Belief Propagation [Processor, Flags ...]")
plt.xlabel('n')
plt.ylabel('Performance [F/C]')
plt.ylim(ymin=0)
plt.ylabel('flops/cycle')
#plt.grid()
ax = plt.gca()
ax.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax.grid(b=True, which='major', color='w', linewidth=1.0)
ax.grid(b=True, which='minor', color='w', linewidth=0.5)
#labelLines(ax.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False, 
#            outline_color=None, 
#            outline_width=0)
box = ax.get_position()
ax.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': 6})
### Generate the plot
plt.savefig(OUT + 'performance_plot.png')
plt.savefig(OUT + 'performance_plot.pdf')
if show:
    plt.show()
plt.close()
