#include <elf_reader.h>

static Elf_Scn *get_elf_section(Elf *elf, char *section) {

    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    size_t shstrndx;

    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        die("(getshdrstrndx) %s", elf_errmsg(-1));
    }

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            die("(getshdr) %s", elf_errmsg(-1));
        }

        if (strcmp(elf_strptr(elf, shstrndx, shdr.sh_name), section) == 0) {
            return scn;
        }
    }

    return NULL;
}

static void check_symtab(Elf *elf, fns_t *fns) {
    Elf_Scn *scn = get_elf_section(elf, ".symtab");
    if (scn == NULL) {
        fns = NULL;
        return;
    }

    Elf_Data *data = NULL;
    data = elf_getdata(scn, NULL);

    GElf_Shdr shdr;
    if (gelf_getshdr(scn, &shdr) != &shdr) {
        die("(getshdr) %s", elf_errmsg(-1));
    }

    int count = shdr.sh_size / shdr.sh_entsize;
    int fn_cnt = 0;

    for (int i = 0; i < count; i++) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        if (ELF64_ST_TYPE(sym.st_info) == STT_FUNC) {
            fn_cnt++;
        }
    }

    fns->addr = malloc((fn_cnt + 1) * sizeof(Elf64_Addr));
    fns->name = malloc((fn_cnt + 1) * sizeof(char *));
    if (fns->addr == NULL || fns->name == NULL) {
        die("allocating function info");
    }
    fns->addr[fn_cnt] = (Elf64_Addr)0;
    fns->name[fn_cnt] = NULL;

    for (int i = 0, j = 0; i < count; i++) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        if (ELF64_ST_TYPE(sym.st_info) == STT_FUNC) {
            fns->addr[j] = sym.st_value;
            fns->name[j] = elf_strptr(elf, shdr.sh_link, sym.st_name);
            j++;
        }
    }
}

static int fd = -1;
static void cleanup(void) {
    int retval = close(fd);
    if (retval == -1) {
        perror("cleanup: close fd");
    }
}

void load_ELF(char *filename, fns_t *fns, bool *pie) {

    Elf *elf;
    /* Initilization.*/
    if (elf_version(EV_CURRENT) == EV_NONE) {
        die("(version) %s", elf_errmsg(-1));
    }

    fd = open(filename, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        perror(filename);
        die("opening file %s", filename);
    }
    atexit(cleanup);

    elf = elf_begin(fd, ELF_C_READ, NULL);
    if (!elf) {
        die("(begin) %s", elf_errmsg(-1));
    }
    Elf64_Ehdr *ehdr = elf64_getehdr(elf);
    if (!ehdr) {
        die("(begin) %s", elf_errmsg(-1));
    }
    uint16_t e_type = ehdr->e_type;
    switch (e_type) {
    case ET_EXEC:
        *pie = false;
        break;
    case ET_DYN:
        *pie = true;
        break;
    default:
        die("(begin) %s", "cannot handle given ELF type");
    }

    check_symtab(elf, fns);
    elf_end(elf);
}
