#!/usr/bin/env python3
"""
    plot performance
"""

import argparse
import os
import glob
from re import X
from pathlib import Path
import pandas as pd
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt
from labellines import labelLines
import itertools
import sys
from matplotlib.ticker import ScalarFormatter
import numpy as np
from scipy import ndimage

ROOT = os.path.join(os.path.dirname(__file__), '../')
parser = argparse.ArgumentParser()
parser.add_argument("measurements", default=[], type=str, nargs="*")
parser.add_argument("-n", "--no-preview", action="store_true", help="Don't preview the graphs, just save them as PNG/PDF")
parser.add_argument("-e", "--ext", type=str, default="png", help="Output file extension (png, pdf)")
parser.add_argument("-o", "--out", type=str, default=ROOT + "/plots", help="Output directory")
parser.add_argument("-r", "--ref", type=str, default=None, help="Reference measurements to which all other measurements will be related.")
parser.add_argument("-p", "--pretty", type=str, default=[], nargs="+", help="Pretty names for the given measurement files.")
args = parser.parse_args()
DPI=300
TITLE = "Belief Propagation [Intel i5-7200U 2.5 GHz, g++ -Ofast -march=native]"
LEGEND_SIZE = 10
ITERATIONS_NORMALISE = 10

peak_perf_scalar = 2
peak_perf_vec = 8
beta = 6 #mem bandwidth bytes/cycles (15GB/s / 2.5 GHz) (https://ark.intel.com/content/www/us/en/ark/products/95443/intel-core-i57200u-processor-3m-cache-up-to-3-10-ghz.html)
I_s = peak_perf_scalar/beta
I_v = peak_perf_vec/beta
intercept_s = peak_perf_scalar - beta*I_s
intercept_v = peak_perf_vec - beta*I_v
L1_beta = 81
I_s_L1 = peak_perf_scalar/L1_beta
I_v_L1 = peak_perf_vec/L1_beta

#bw from https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf
mem_bottlenecks = [
    {"name" : 'mem ' + r'$\beta$', "bw": 6},
    {"name" : 'L3 ' + r'$\beta$', "bw": 18},
    {"name" : 'L2 ' + r'$\beta$', "bw": 29},
    {"name" : 'L1 ' + r'$\beta$', "bw": 81},
]

just_prop = True

def cycle_markers(iterable,n):
  for item in itertools.cycle(iterable):
    yield item

def read_csv(file):
    df = pd.read_csv(file, usecols= list(range(16)), index_col=False)
    df = df.sort_values(df.columns[0])
    return df.iloc[:].to_numpy()

def split_to_cols(matrix):
    out = []
    for i in range(matrix.shape[1]):
        out.append(matrix[:,i])
    return out

sns.set_theme()
markers = cycle_markers(('^','o','s'),1)

OUT = args.out + "/"
if not args.measurements:
    args.measurements = glob.glob(f"{ROOT}/measurements/small/*.csv")
args.pretty += [None] * (len(args.measurements) - len(args.pretty))
fileNames = sorted(zip(args.measurements, args.pretty))

perf_limits = [.5, 8]
intensity_limits = [.01, 2]

perfFigure = plt.figure(1)
ax1 = perfFigure.gca()
rtFigure = plt.figure(2)
ax2 = rtFigure.gca()
cyclesVsFlopsFigure = plt.figure(3)
ax3 = cyclesVsFlopsFigure.gca()
propRtFigure = plt.figure(4)
ax4 = propRtFigure.gca()
if args.ref:
    cyclesVsReference = plt.figure(5)
    ax5 = cyclesVsReference.gca()
rooflineFigure = plt.figure(6)
ax6 = rooflineFigure.gca()

if args.ref:
    table = read_csv(args.ref)
    total_cycles_reference = split_to_cols(table)[4 if just_prop else 1]

for file, pretty_name in fileNames:
    table = read_csv(file)
    input_sizes, total_cycles, total_flops, gbuild_cycles, prop_cycles, bel_cycles, gbuild_flops, prop_flops, bel_flops, iterations, _, _, _, databytes_contiguous, databytes_random, sizeof_random = split_to_cols(table)
    prop_flops *= ITERATIONS_NORMALISE / iterations
    prop_cycles *= ITERATIONS_NORMALISE / iterations
    if just_prop:
        total_flops = prop_flops
        total_cycles = prop_cycles
    flops_per_cycle = total_flops / total_cycles
    if not input_sizes.size:
        # the corresponding run has probably crashed, if we don't ignore it, we run into problems later
        continue
    marker = next(markers)
    if pretty_name is None:
        pretty_name = Path(file).stem.replace("@", " ").replace("__", "=")
    ax1.plot(input_sizes, flops_per_cycle, label = pretty_name, marker=marker)
    ax2.plot(input_sizes, total_cycles, label = pretty_name, marker=marker)
    ax3.plot(total_cycles, total_flops, label = pretty_name, marker=marker)

    pcyclesPerIt = (prop_cycles + bel_cycles)/ iterations
    ax4.plot(input_sizes, pcyclesPerIt, label = pretty_name, marker=marker)

    if args.ref:
        ax5.plot(input_sizes, total_cycles_reference[:total_cycles.size] / total_cycles, label=pretty_name, marker=marker)
    
    ### ROOFLINE ###
    if np.any(sizeof_random == 1):
        continue # with_library
    databytes = databytes_contiguous + 64/sizeof_random*databytes_random #remove 64/sizeof_random for lowest data access bound
    operational_intensity = total_flops/databytes
    performance = total_flops/total_cycles
    final_perf = [x if operational_intensity[i] > x/beta else operational_intensity[i]*beta for i, x in enumerate(performance)]
    ax6.plot(operational_intensity, final_perf, label=pretty_name, marker=marker, markersize=2, linewidth=1)
    perf_limits[0] = min(perf_limits[0], min(final_perf))
    perf_limits[1] = max(perf_limits[1], max(final_perf))
    intensity_limits[0] = min(intensity_limits[0], min(operational_intensity))
    intensity_limits[1] = max(intensity_limits[1], max(operational_intensity))
    #plot first point/smallest n with different color
    ax6.plot(operational_intensity[0], final_perf[0], marker=marker, markersize=2, color='black', linewidth=1)

try:
    os.mkdir(OUT)
except FileExistsError:
    os.utime(OUT) # touch for make
    pass

ax1.set_title(TITLE)
ax1.set_xlabel('n')
ax1.set_ylim(ymin=0)
ax1.set_ylabel('flops/cycle')
#ax1.grid()
ax1.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax1.grid(visible=True, which='major', color='w', linewidth=1.0)
ax1.grid(visible=True, which='minor', color='w', linewidth=0.5)
# labelLines(ax1.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False,
#            outline_color=None, 
#            outline_width=0)
box = ax1.get_position()
ax1.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax1.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': LEGEND_SIZE})
### Generate the plot
perfFigure.savefig(f'{OUT}/performance_plot.{args.ext}', dpi=DPI)

ax2.set_title(TITLE)
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
ax2.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': LEGEND_SIZE})
### Generate the plot
rtFigure.savefig(f'{OUT}/runtime_plot.{args.ext}', dpi=DPI)

ax3.set_title(TITLE)
ax3.set_xlabel('cycles')
ax3.set_ylim(ymin=0)
ax3.set_ylabel('flops')
#ax3.grid()
ax3.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax3.grid(visible=True, which='major', color='w', linewidth=1.0)
ax3.grid(visible=True, which='minor', color='w', linewidth=0.5)
# labelLines(ax3.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False, 
#            outline_color=None, 
#            outline_width=0)
box = ax3.get_position()
ax3.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax3.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': LEGEND_SIZE})
### Generate the plot
cyclesVsFlopsFigure.savefig(f'{OUT}/cyclesVsFlops_plot.{args.ext}', dpi=DPI)

ax4.set_title(TITLE)
ax4.set_xlabel('n')
#ax4.set_ylim(ymin=0)
ax4.set_ylabel('cycles per iteration')
ax4.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax4.grid(visible=True, which='major', color='w', linewidth=1.0)
ax4.grid(visible=True, which='minor', color='w', linewidth=0.5)
box = ax4.get_position()
ax4.set_position([box.x0, box.y0 + box.height * 0.3,
                 box.width, box.height * 0.7])
ax4.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': LEGEND_SIZE})
ax4.set_yscale('log')
### Generate the plot
rtFigure.savefig(f'{OUT}/runtime_plot.{args.ext}', dpi=DPI)


if args.ref:
    ax5.set_title(TITLE)
    ax5.set_xlabel('n')
    #ax5.set_ylim(ymin=0)
    ax5.set_ylabel('speedup w.r.t. reference') # TODO
    ax5.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax5.grid(visible=True, which='major', color='w', linewidth=1.0)
    ax5.grid(visible=True, which='minor', color='w', linewidth=0.5)
    box = ax5.get_position()
    ax5.set_position([box.x0, box.y0 + box.height * 0.3,
                     box.width, box.height * 0.7])
    ax5.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': LEGEND_SIZE})
    ### Generate the plot
    cyclesVsReference.savefig(f'{OUT}/cyclesVsReference_plot.{args.ext}', dpi=DPI)

intensity_limits[0] /= 2
intensity_limits[1] *= 2
xlogsize = float(np.log10(intensity_limits[1]/intensity_limits[0]))
ylogsize = float(np.log10(perf_limits[1]/perf_limits[0]))
m = xlogsize/ylogsize

# draw rooflines
ax6.plot((I_s_L1, I_s + 100), (peak_perf_scalar, peak_perf_scalar),'--', color='black', linewidth=0.5)
ax6.plot((I_v_L1, I_v + 100), (peak_perf_vec, peak_perf_vec), '--', color='black', linewidth=0.5)
ax6.text( intensity_limits[1]/(10**(xlogsize*0.01)), peak_perf_scalar*(10**(ylogsize*0.03)),'Scalar Roofline', ha='right', fontsize=5, color='black')
ax6.text( intensity_limits[1]/(10**(xlogsize*0.01)), peak_perf_vec*(10**(ylogsize*0.03)),'Vector Roofline', ha='right', fontsize=5, color='black')

# draw slopes
for slope in mem_bottlenecks:
    y_s = [0,peak_perf_vec]
    x_s = [float(y)/slope["bw"] for y in y_s]
    ax6.plot(x_s, y_s, '--',color='black', linewidth=0.5)
    #label
    xpos = intensity_limits[0]*(10**(xlogsize*0.04))
    ypos = xpos*slope['bw']
    if ypos<perf_limits[0]:
        ypos =  perf_limits[0]*(10**(ylogsize*0.02))
        xpos =  ypos/slope['bw']
    pos = (xpos,ypos)
    ax6.annotate(slope['name'] + ': ' + str(slope['bw']) + " B/c", pos, rotation=np.arctan(m/2)*180/np.pi, rotation_mode="anchor",
    fontsize=5,
    ha="left", va='bottom',
    color='black')

ax6.set_title(TITLE)
ax6.set_xlabel('I(n) [flops/byte]')
ax6.set_ylabel('P(n) [flops/cycle]')
ax6.set_ylim(ymin=0)
ax6.get_xaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
ax6.grid(visible=True, which='major', color='w', linewidth=1.0)
ax6.grid(visible=True, which='minor', color='w', linewidth=0.5)
# labelLines(ax6.get_lines(), 
#            yoffsets=0.022, 
#            fontsize=10, 
#            align=False, 
#            outline_color=None, 
#            outline_width=0)
box = ax6.get_position()
ax6.set_position([box.x0, box.y0 + box.height * 0.3,
                     box.width, box.height * 0.7])
ax6.legend(loc='upper center', bbox_to_anchor=(0.5, -0.2), ncol=1,prop={'size': LEGEND_SIZE})
ax6.set_xscale('log', base=10)
ax6.set_yscale('log', base=10)
yticks = [2,8]
ax6.set_yticks(yticks)
ax6.xaxis.set_major_formatter(ScalarFormatter())
ax6.yaxis.set_major_formatter(ScalarFormatter())
perf_limits[0] /= 2
perf_limits[1] *= 2
ax6.set_xlim(intensity_limits)
ax6.set_ylim(perf_limits)
### Generate the plot
rooflineFigure.savefig(f'{OUT}/roofline_plot.{args.ext}', dpi=DPI)

if not args.no_preview:
    plt.show()
plt.close()
