from tabulate import tabulate


table = [['Nome', 'Duracao Total', 'Duracao Preditor(ms)', 'Duracao Histograma(ms)', 'Duracao Codificar(ms)', 
'Duracao Descodificar(ms)', 'M','Original(kB)', 'Comprimido(kB)', 'Compression'] ]

info = []
for n in range(1,2):
	f = open(str(n)+".prop", "r")
	for i,x in enumerate(f):
		if int(i) == 0 :
			name = x.split(";;:;")[0].split("/")[1]
			info.append(name)
			info = info+x.split(";;:;")[1].split("\t")[:-1]
		else:
			info.append(int(x)/8000)
	info.append("{:.2f} %".format(info[-2]/info[-1]*100))	
	table.append(info)
	info = []

print(tabulate(table, headers='firstrow', tablefmt='fancy_grid'))