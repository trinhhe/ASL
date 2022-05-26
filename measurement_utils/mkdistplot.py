import sys
import matplotlib.pyplot as plt
import seaborn as sns

x, y = zip(*(map(int, line.split()) for line in sys.stdin.readlines()))

sns.kdeplot(x, weights=y, log_scale=True)
plt.savefig(sys.argv[1])
