import m5
from m5.objects import *
from new_cache import *
from optparse import OptionParser

parser = OptionParser()
parser.add_option('--fb', action="store_true")
parser.add_option('--random', action="store_true")
parser.add_option('--lru', action="store_true")
parser.add_option('--mru', action="store_true")
parser.add_option('--fifo', action="store_true")
parser.add_option('--tplru', action="store_true")
parser.add_option('--wlru', action="store_true")

parser.add_option('--twomm', action="store_true")
parser.add_option('--bfs', action="store_true")
parser.add_option('--bzip2', action="store_true")
parser.add_option('--mcf', action="store_true")

parser.add_option('--l1i_size', type="string", default="32kB")
parser.add_option('--l1i_assoc', type="int", default=4)
parser.add_option('--l1i_resl', type="int", default=2)
parser.add_option('--l1i_hitl', type="int", default=2)
parser.add_option('--l1d_size', type="string", default="32kB")
parser.add_option('--l1d_assoc', type="int", default=4)
parser.add_option('--l1d_resl', type="int", default=2)
parser.add_option('--l1d_hitl', type="int", default=2)
parser.add_option('--l2_size', type="string", default="2MB")
parser.add_option('--l2_assoc', type="int", default=8)
parser.add_option('--l2_resl', type="int", default=20)
parser.add_option('--l2_hitl', type="int", default=20)

(options, args) = parser.parse_args()

root = Root()
root.full_system = False
root.system = System()

root.system.clk_domain = SrcClockDomain()
root.system.clk_domain.clock = '2GHz'
root.system.clk_domain.voltage_domain = VoltageDomain()

root.system.mem_mode = 'timing'
root.system.mem_ranges = [AddrRange ('2048MB')]
root.system.mem_ctrl = DDR3_1600_8x8()
root.system.mem_ctrl.range = root.system.mem_ranges[0]

root.system.cpu = DerivO3CPU()

root.system.cpu.icache = L1ICache()
root.system.cpu.icache.size = options.l1i_size
root.system.cpu.icache.assoc = options.l1i_assoc
root.system.cpu.icache.response_latency = options.l1i_resl
root.system.cpu.icache.tag_latency = options.l1i_hitl
root.system.cpu.icache.data_latency = options.l1i_hitl

root.system.cpu.dcache = L1DCache()
root.system.cpu.dcache.size = options.l1d_size
root.system.cpu.dcache.assoc = options.l1d_assoc
root.system.cpu.dcache.response_latency = options.l1d_resl
root.system.cpu.dcache.tag_latency = options.l1d_hitl
root.system.cpu.dcache.data_latency = options.l1d_hitl

root.system.l2cache = L2Cache()
root.system.l2cache.size = options.l2_size
root.system.l2cache.assoc = options.l2_assoc
root.system.l2cache.response_latency = options.l2_resl
root.system.l2cache.tag_latency = options.l2_hitl
root.system.l2cache.data_latency = options.l2_hitl

if options.fb:
	root.system.cpu.icache.replacement_policy = FBRP()
	root.system.cpu.dcache.replacement_policy = FBRP()
	root.system.l2cache.replacement_policy = FBRP()
elif options.random:
	root.system.cpu.icache.replacement_policy = RandomRP()
	root.system.cpu.dcache.replacement_policy = RandomRP()
	root.system.l2cache.replacement_policy = RandomRP()
elif options.lru:
	root.system.cpu.icache.replacement_policy = LRURP()
	root.system.cpu.dcache.replacement_policy = LRURP()
	root.system.l2cache.replacement_policy = LRURP()
elif options.mru:
	root.system.cpu.icache.replacement_policy = MRURP()
	root.system.cpu.dcache.replacement_policy = MRURP()
	root.system.l2cache.replacement_policy = MRURP()
elif options.fifo:
	root.system.cpu.icache.replacement_policy = FIFORP()
	root.system.cpu.dcache.replacement_policy = FIFORP()
	root.system.l2cache.replacement_policy = FIFORP()
elif options.tplru:
	root.system.cpu.icache.replacement_policy = TreePLRURP()
	root.system.cpu.dcache.replacement_policy = TreePLRURP()
	root.system.l2cache.replacement_policy = TreePLRURP()
elif options.wlru:
	root.system.cpu.icache.replacement_policy = WeightedLRURP()
	root.system.cpu.dcache.replacement_policy = WeightedLRURP()
	root.system.l2cache.replacement_policy = WeightedLRURP()

root.system.membus = SystemXBar()

root.system.cpu.icache.cpu_side = root.system.cpu.icache_port
root.system.cpu.dcache.cpu_side = root.system.cpu.dcache_port

root.system.l2bus = L2XBar()
root.system.cpu.icache.mem_side = root.system.l2bus.slave
root.system.cpu.dcache.mem_side = root.system.l2bus.slave
root.system.l2cache.cpu_side = root.system.l2bus.master
root.system.l2cache.mem_side = root.system.membus.slave

root.system.mem_ctrl.port = root.system.membus.master
root.system.cpu.createInterruptController()
root.system.system_port = root.system.membus.slave
# root.system.cpu.interrupt[0].pio = root.system.membus.master
# root.system.cpu.interrupt[0].int_master = root.system.membus.slave
# root.system.cpu.interrupt[0].int_slave = root.system.membus.master

root.system.cpu.max_insts_any_thread = 5w00000000

process = Process()

if options.twomm:
	process.cmd = ['/home/jaehan/gem5/test_bench/2MM/2mm_base']
elif options.bfs:
	process.cmd = ['/home/jaehan/gem5/test_bench/BFS/bfs','-f','/home/jaehan/gem5/test_bench/BFS/USA-road-d.NY.gr']
elif options.bzip2:
	process.cmd = ['/home/jaehan/gem5/test_bench/bzip2/bzip2_base.amd64-m64-gcc42-nn','/home/jaehan/gem5/test_bench/bzip2/input.source','280']
elif options.mcf:
	process.cmd = ['/home/jaehan/gem5/test_bench/mcf/mcf_base.amd64-m64-gcc42-nn','/home/jaehan/gem5/test_bench/mcf/inp.in']
else:
	process.cmd = ['/home/jaehan/gem5/tests/test-progs/hello/bin/arm/linux/hello']

root.system.cpu.workload = process
root.system.cpu.createThreads()

m5.instantiate()
exit_event = m5.simulate()
print ('Existing @ tick', m5.curTick(), 'because', exit_event.getCause())
