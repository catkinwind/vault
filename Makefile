CC = gcc

SOURCES = \
  enc \
	entry \
	hash \
	editor \
	vault

TESTS = \
	enc \
	enc_test

LIBS = -lcrypto

debug   : CFLAGS = -gdwarf-2
debug   : vault test

.PHONY: all debug clean OUTDIR

SRC_DIR = src
OUTDIR = bin
OBJ_OUTDIR = obj

OBJS := $(addprefix $(OBJ_OUTDIR)/, $(addsuffix .o, $(SOURCES)))
TESTS := $(addprefix $(OBJ_OUTDIR)/, $(addsuffix .o, $(TESTS)))

default: vault

all: vault test

vault: $(OUTDIR) $(OBJ_OUTDIR) $(OBJS)
	$(CC) -o $(OUTDIR)/vault $(OBJS) $(LIBS)

test: $(OUTDIR) $(OBJ_OUTDIR) $(TESTS)
	$(CC) -o $(OUTDIR)/test $(TESTS) $(LIBS)

$(OUTDIR):
	mkdir -p $@

$(OBJ_OUTDIR):
	mkdir -p $@

$(OBJ_OUTDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	-rm -rf $(OBJ_OUTDIR)
	-rm -rf $(OUTDIR)
