import matplotlib.pyplot as plt
import numpy as np
import sys
# An "interface" to matplotlib.axes.Axes.hist() method
value = []
occurences = []
d = []


f = open(sys.argv[1], "r")
for x in f:
  values = x.split("\t")
  value.append(values[0])
  occurences.append(values[1])
  print(values[0])



fig = plt.figure()
ax = fig.add_axes([0.1,0.1,0.9,0.9])

ax.bar(value,occurences)
plt.show()
