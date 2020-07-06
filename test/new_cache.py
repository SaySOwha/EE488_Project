from m5.objects import *

class L1Cache(Cache):
	size = '32kB'
	assoc = 4
	tag_latency = 2
	data_latency = 2
	response_latency = 2
	mshrs = 4
	tgts_per_mshr = 20

class L1DCache(Cache):
	size = '32kB'
	assoc = 4
	tag_latency = 2
	data_latency = 2
	response_latency = 2
	mshrs = 4
	tgts_per_mshr = 20

class L1ICache(Cache):
	size = '32kB'
	assoc = 4
	tag_latency = 2
	data_latency = 2
	response_latency = 2
	mshrs = 4
	tgts_per_mshr = 20

class L2Cache(Cache):
	size = '2MB'
	assoc = 8
	tag_latency = 20
	data_latency = 20
	response_latency = 20
	mshrs = 20
	tgts_per_mshr = 12

class L3Cache(Cache):
	size = '8MB'
	assoc = 16
	tag_latency = 32
	data_latency = 32
	response_latency = 32
	mshrs = 32
	tgts_per_mshr = 12