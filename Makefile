###########################################
# Makefile for libskconntest.so
#
#

LIB_NAME := skconncheck
SRC_DIR := src
INC_DIR := inc

TEST_APP := sk_test
TEST_SRC_DIR := test

ifeq ($(shell which curl-config),)
$(error "No curl-config in $(PATH), please install libcurl")
endif

CC := gcc
CFLAGS := -Wall -Wextra
ifeq ($(ENABLE_OPTIMISATIONS), 1)
CFLAGS += -O3
endif
ifeq ($(DEBUG_BUILD),1)
CFLAGS += -g
CFLAGS += -DDEBUG_BUILD
endif

LIB_LDFLAGS := -shared -lcurl
LIB_CFLAGS := -fPIC

TEST_LDFLAGS := -Wl,-rpath=. -lskconncheck -lcurl

.PHONY: all clean

all: $(LIB_NAME) $(TEST_APP)

$(LIB_NAME): $(SRC_DIR)/$(LIB_NAME).c $(INC_DIR)/$(LIB_NAME).h
	$(CC) $(CFLAGS) -o lib$@.so $^ -I$(INC_DIR) $(LIB_CFLAGS) $(LIB_LDFLAGS)

$(TEST_APP): $(TEST_SRC_DIR)/$(TEST_APP).c $(INC_DIR)/$(LIB_NAME).h
	$(CC) $(CFLAGS) -o $@ $^ -I$(INC_DIR) -L. $(TEST_LDFLAGS)
	
clean:
	$(RM) lib$(LIB_NAME).so $(TEST_APP)
