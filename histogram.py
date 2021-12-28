import matplotlib.pyplot as plt
import numpy as np
import sys
# An "interface" to matplotlib.axes.Axes.hist() method
value = []
occurences = []
d = []
colors = []


f = open(sys.argv[1], "r")
for x in f:
  values = x.split("\t")
  value.append(int(values[0]))
  occurences.append(int(values[1]))
  # print(values[0])

  
maximo = max(occurences)
print(value[occurences.index(maximo)])
print(maximo)
for occ in occurences:
  
  if int(occ) > maximo*3/4:
    colors.append("red")
  elif int(occ)>maximo/2:
    colors.append("green")
  elif int(occ)>maximo/4:
    colors.append("blue")
  else:
    colors.append("yellow")



fig = plt.figure()
ax = fig.add_axes([0.075,0.05,0.9,0.9])

ax.bar(value,occurences, color=colors)

plt.yscale('linear')
plt.xscale('linear')
# plt.xlim([-32728,32727])
plt.xlim([min(value), max(value)])
plt.show()
