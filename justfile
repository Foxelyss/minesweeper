# Prepare build commands for autocompletion(language server providers)
prepare_for_lsp:
	scons compiledb=yes

# Build game
build platform dev=("no"):
	@echo Platform is {{platform}} With dev = {{if dev != "no" {"yes"} else {"no"}}}
	scons platform={{platform}} {{if dev != "no" {"target=template_release "} else {"target=template_debug dev_build=yes"}}}
