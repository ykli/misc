# config.mk

%.o:%.c
	@echo "  [CC]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(Q)$(CC) -c $(CFLAGS) $< -o $@

%.o:%.cpp
	@echo "  [CC]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(Q)$(CPLUSPLUS) -c $(CFLAGS) $< -o $@
