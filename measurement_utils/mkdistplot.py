import sys
import matplotlib.pyplot as plt
import seaborn as sns

x, y = zip(*(map(int, line.split()) for line in sys.stdin.readlines()))
cnt = 0.
wcnt = 0.
for i, j in zip(x, y):
    cnt += i
    wcnt += i * j

print(wcnt / cnt)

sns.kdeplot(x, weights=y, log_scale=True)
plt.savefig(sys.argv[1])
