TOP=$(top_srcdir)

NAME = managed
OUTDIR = bin/Debug
ASSEMBLY = $(NAME).dll

outputs = \
	$(ASSEMBLY) \
	$(NAME).h \
	$(NAME).g.c \
	dnne.h

$(OUTDIR)/$(ASSEMBLY): $(NAME).csproj *.cs
	dotnet build $<

$(ASSEMBLY): $(OUTDIR)/$(ASSEMBLY)
	$(CP) $< $@

$(NAME).h: $(OUTDIR)/$(NAME).h
	$(CP) $< $@

dnne.h: $(OUTDIR)/dnne.h
	$(CP) $< $@

$(NAME).g.c: $(OUTDIR)/$(NAME).g.c
	$(CP) $< $@

clean:
	$(RM_R) bin
	$(RM_R) obj
	$(RM) $(outputs)

all: $(outputs)