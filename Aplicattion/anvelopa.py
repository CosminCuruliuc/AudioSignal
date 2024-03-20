import numpy as np
from scipy.signal import hilbert

with open("waveData.txt", "r") as f:
	signal = [int(line.lstrip()) for line in f.readlines()]
analytic_signal = hilbert(signal) #signal este vectorul aferent semnalului analizat
amplitude_envelope = np.abs(analytic_signal) #amplitude_envelope este anvelopa semnalului

with open("anvelopa.txt", "w") as f:
	for sample in amplitude_envelope:
		f.write(f"{sample}\n")