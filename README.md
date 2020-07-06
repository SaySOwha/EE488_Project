Gem5 is available with:

	https://github.com/gem5/gem5

Deploy new replacement policy:

	git clone https://github.com/rlawogks145/EE488_Project.git

	cd EE488_Project

	mv lw_rp.cc  lw_rp.hh  ReplacementPolicies.py  SConscript <Gem5 path>/src/mem/cache/replacement_policies


Then, rebuild gem5 and you can use the replacement policy like below, note the reference code test/two_level.py

	root.system.l2cache.replacement_policy = LWRP()