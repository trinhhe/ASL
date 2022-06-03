#!/usr/bin/env python3
"""
    plot performance
"""

import argparse
import os
import glob
from re import X
from pathlib import Path
from jinja2 import PrefixLoader
import pandas as pd
from pytest import mark
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt
from labellines import labelLines
import itertools
import sys

from sympy import Q

ROOT = os.path.join(os.path.dirname(__file__), '../')
parser = argparse.ArgumentParser()
parser.add_argument("measurements", default=[], type=str, nargs="*")
parser.add_argument("-n", "--no-preview", action="store_true", help="Don't preview the graphs, just save them as PNG/PDF")
parser.add_argument("-e", "--ext", type=str, default="png", help="Output file extension (png, pdf)")
parser.add_argument("-o", "--out", type=str, default=ROOT + "/plots", help="Output directory")
args = parser.parse_args()

def cycle_markers(iterable,n):
  for item in itertools.cycle(iterable):
    yield item

sns.set_theme()
markers = cycle_markers(('^','o','s'),1)

OUT = args.out + "/"
try:
    os.mkdir(OUT)
except FileExistsError:
    pass

if not args.measurements:
    args.measurements = glob.glob(f"{ROOT}/measurements/*.csv")
fileNames = sorted(args.measurements)

perfFigure = plt.figure(1)
ax1 = perfFigure.gca()
rtFigure = plt.figure(2)
ax2 = rtFigure.gca()
cyclesVsFlopsFigure = plt.figure(3)
ax3 = cyclesVsFlopsFigure.gca()
propRtFigure = plt.figure(4)
ax4 = propRtFigure.gca()

for file in fileNames:
    df = pd.read_csv(file, usecols= [0,1,2,3,4,5,9], index_col=False)
    df = df.sort_values(df.columns[0])
    input_sizes = df.iloc[:,0].to_numpy()
    total_cycles = df.iloc[:,1].to_numpy()
    total_flops = df.iloc[:,2].to_numpy()
    gbuild_cycles = df.iloc[:,3].to_numpy()
    prop_cycles = df.iloc[:,4].to_numpy()
    bel_cycles = df.iloc[:,5].to_numpy()
    interations = df.iloc[:,6].to_numpy()
    flops_per_cycle = total_flops / total_cycles
    if not input_sizes.size:
        # the corresponding run has probably crashed, if we don't ignore it, we run into problems later
        continue
    marker = next(markers)
    pretty_name = Path(file).stem.replace("@", " ").replace("__", "=")
    ax1.plot(input_sizes, flops_per_cycle, label = pretty_name, marker=marker)
    ax2.plot(input_sizes, total_cycles, label = pretty_name, marker=marker)
    ax3.plot(total_cycles, total_flops, label = pretty_name, marker=marker)

    pcyclesPerIt = (prop_cycles + bel_cycles)/ interations
    ax4.plot(input_sizes, pcyclesPerIt, label = pretty_name, marker=marker)

ax1.set_title("Belief Propagation [Processor, Flags ...]")
ax1.set_xlabel('n')
ax1.set_ylim(ymin=0)
ax1.set_ylabel('flops/cycle')
#ax1.grid()
ax1.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax1.grid(visible=True, which='major', color='w', linewidth=1.0)
ax1.grid(visible=True, which='minor', color='w', linewidth=0.5)
#labelLines(ax1.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False, 
#            outline_color=None, 
#            outline_width=0)
box = ax1.get_position()
ax1.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax1.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': 6})
### Generate the plot
perfFigure.savefig(f'{OUT}/performance_plot.{args.ext}')

ax2.set_title("Belief Propagation [Processor, Flags ...]")
ax2.set_xlabel('n')
ax2.set_ylim(ymin=0)
ax2.set_ylabel('cycles')
#ax2.grid()
ax2.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax2.grid(visible=True, which='major', color='w', linewidth=1.0)
ax2.grid(visible=True, which='minor', color='w', linewidth=0.5)
#labelLines(ax2.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False, 
#            outline_color=None, 
#            outline_width=0)
box = ax2.get_position()
ax2.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax2.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': 6})
### Generate the plot
rtFigure.savefig(f'{OUT}/runtime_plot.{args.ext}')

ax3.set_title("Belief Propagation [Processor, Flags ...]")
ax3.set_xlabel('cycles')
ax3.set_ylim(ymin=0)
ax3.set_ylabel('flops')
#ax3.grid()
ax3.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax3.grid(visible=True, which='major', color='w', linewidth=1.0)
ax3.grid(visible=True, which='minor', color='w', linewidth=0.5)
#labelLines(ax3.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False, 
#            outline_color=None, 
#            outline_width=0)
box = ax3.get_position()
ax3.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax3.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': 6})
### Generate the plot
cyclesVsFlopsFigure.savefig(f'{OUT}/cyclesVsFlops_plot.{args.ext}')

ax4.set_title("Belief Propagation [Processor, Flags ...]")
ax4.set_xlabel('n')
#ax4.set_ylim(ymin=0)
ax4.set_ylabel('cycles per iteration')
ax4.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax4.grid(visible=True, which='major', color='w', linewidth=1.0)
ax4.grid(visible=True, which='minor', color='w', linewidth=0.5)
box = ax4.get_position()
ax4.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax4.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': 6})
ax4.set_yscale('log')
### Generate the plot
rtFigure.savefig(f'{OUT}/runtime_plot.{args.ext}')


if not args.no_preview:
    plt.show()
plt.close()
