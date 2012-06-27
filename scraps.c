
#line 929 "nuweb.w"
#include "global.h"

#line 4163 "nuweb.w"
#define SLAB_SIZE 1024

typedef struct slab {
  struct slab *next;
  char chars[SLAB_SIZE];
} Slab;

#line 4172 "nuweb.w"
typedef struct {
  char *file_name;
  Slab *slab;
  struct uses *uses;
  struct uses *defs;
  int file_line;
  int page;
  char letter;
  unsigned char sector;
} ScrapEntry;

#line 4185 "nuweb.w"
static ScrapEntry *SCRAP[256];

#define scrap_array(i) SCRAP[(i) >> 8][(i) & 255]

static int scraps;
int num_scraps()
{
   return scraps;
};
/* Forward declarations for scraps.c */

#line 4706 "nuweb.w"
int delayed_indent = 0;

#line 4989 "nuweb.w"
static void
comment_ArglistElement(FILE * file, Arglist * args, int quote)
{
  Name *name = args->name;
  Arglist *q = args->args;

  if (name == NULL) {
    if (quote)
       fprintf(file, "%c'%s%c'", nw_char, (char *)q, nw_char);
    else
       fprintf(file, "'%s'", (char *)q);
  } else if (name == (Name *)1) {
     /* Include an embedded scrap in comment */
     
#line 5009 "nuweb.w"
     Embed_Node * e = (Embed_Node *)q;
     fputc('{', file);
     write_scraps(file, "", e->defs, -1, "", 0, 0, 0, 0, e->args, 0, 1, "");
     fputc('}', file);
#line 5001 "nuweb.w"

  } else {
     /* Include a fragment use in comment */
     
#line 5015 "nuweb.w"
     char * p = name->spelling;
     if (quote)
        fputc(nw_char, file);
     fputc('<', file);
     if (quote && name->sector == 0)
        fputc('+', file);
     while (*p != '\000') {
       if (*p == ARG_CHR) {
         comment_ArglistElement(file, q, quote);
         q = q->next;
         p++;
       }
       else
          fputc(*p++, file);
     }
     if (quote)
        fputc(nw_char, file);
     fputc('>', file);
#line 5003 "nuweb.w"

  }
}

#line 5504 "nuweb.w"
char * comment_begin[5] = { "", "/* ", "// ", "# ", ";; "};
char * comment_mid[5] = { "", " * ", "// ", "# ", ";; "};
char * comment_end[5] = { "", " */", "", "", ""};

#line 6271 "nuweb.w"

static void add_uses();
static int scrap_is_in();

#line 4194 "nuweb.w"


#line 4209 "nuweb.w"
void init_scraps()
{
  scraps = 1;
  SCRAP[0] = (ScrapEntry *) arena_getmem(256 * sizeof(ScrapEntry));
}

#line 4217 "nuweb.w"
void write_scrap_ref(file, num, first, page)
     FILE *file;
     int num;
     int first;
     int *page;
{
  if (scrap_array(num).page >= 0) {
    if (first!=0)
      fprintf(file, "%d", scrap_array(num).page);
    else if (scrap_array(num).page != *page)
      fprintf(file, ", %d", scrap_array(num).page);
    if (scrap_array(num).letter > 0)
      fputc(scrap_array(num).letter, file);
  }
  else {
    if (first!=0)
      putc('?', file);
    else
      fputs(", ?", file);
    /* Warn (only once) about needing to rerun after Latex */
    
#line 4255 "nuweb.w"
    {
      if (!already_warned) {
        fprintf(stderr, "%s: you'll need to rerun nuweb after running latex\n",
                command_name);
        already_warned = TRUE;
      }
    }
#line 4236 "nuweb.w"

  }
  if (first>=0)
  *page = scrap_array(num).page;
}

#line 4244 "nuweb.w"
void write_single_scrap_ref(file, num)
     FILE *file;
     int num;
{
  int page;
  write_scrap_ref(file, num, TRUE, &page);
}

#line 4272 "nuweb.w"
typedef struct {
  Slab *scrap;
  Slab *prev;
  int index;
} Manager;

#line 4282 "nuweb.w"
static void push(c, manager)
     char c;
     Manager *manager;
{
  Slab *scrap = manager->scrap;
  int index = manager->index;
  scrap->chars[index++] = c;
  if (index == SLAB_SIZE) {
    Slab *new = (Slab *) arena_getmem(sizeof(Slab));
    scrap->next = new;
    manager->scrap = new;
    index = 0;
  }
  manager->index = index;
}

#line 4300 "nuweb.w"
static void pushs(s, manager)
     char *s;
     Manager *manager;
{
  while (*s)
    push(*s++, manager);
}

#line 4310 "nuweb.w"
int collect_scrap()
{
  int current_scrap, lblseq = 0;
  int depth = 1;
  Manager writer;
  /* Create new scrap, managed by \verb|writer| */
  
#line 4321 "nuweb.w"
  {
    Slab *scrap = (Slab *) arena_getmem(sizeof(Slab));
    if ((scraps & 255) == 0)
      SCRAP[scraps >> 8] = (ScrapEntry *) arena_getmem(256 * sizeof(ScrapEntry));
    scrap_array(scraps).slab = scrap;
    scrap_array(scraps).file_name = save_string(source_name);
    scrap_array(scraps).file_line = source_line;
    scrap_array(scraps).page = -1;
    scrap_array(scraps).letter = 0;
    scrap_array(scraps).uses = NULL;
    scrap_array(scraps).defs = NULL;
    scrap_array(scraps).sector = current_sector;
    writer.scrap = scrap;
    writer.index = 0;
    current_scrap = scraps++;
  }
#line 4315 "nuweb.w"

  /* Accumulate scrap and return \verb|scraps++| */
  
#line 4340 "nuweb.w"
  {
    int c = source_get();
    while (1) {
      switch (c) {
        case EOF: fprintf(stderr, "%s: unexpect EOF in (%s, %d)\n",
                          command_name, scrap_array(current_scrap).file_name,
                          scrap_array(current_scrap).file_line);
                  exit(-1);
        default:
          if (c==nw_char)
            {
              /* Handle at-sign during scrap accumulation */
              
#line 4362 "nuweb.w"
              {
                c = source_get();
                switch (c) {
                  case '(':
                  case '[':
                  case '{': depth++;
                            break;
                  case '+':
                  case '-':
                  case '*':
                  case '|': 
#line 4429 "nuweb.w"
                            {
                              do {
                                int type = c;
                                do {
                                  char new_name[MAX_NAME_LEN];
                                  char *p = new_name;
                                  unsigned int sector = 0;
                                  do
                                    c = source_get();
                                  while (isspace(c));
                                  if (c != nw_char) {
                                    Name *name;
                                    do {
                                      *p++ = c;
                                      c = source_get();
                                    } while (c != nw_char && !isspace(c));
                                    *p = '\0';
                                    switch (type) {
                                    case '*':
                                       sector = current_sector;
                                       /* Add user identifier use */
                                       
#line 4473 "nuweb.w"
                                       name = name_add(&user_names, new_name, sector);
                                       if (!name->uses || name->uses->scrap != current_scrap) {
                                         Scrap_Node *use = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                         use->scrap = current_scrap;
                                         use->next = name->uses;
                                         name->uses = use;
                                         add_uses(&(scrap_array(current_scrap).uses), name);
                                       }
#line 4449 "nuweb.w"
                            
                                       break;
                                    case '-':
                                       /* Add user identifier use */
                                       
#line 4473 "nuweb.w"
                                       name = name_add(&user_names, new_name, sector);
                                       if (!name->uses || name->uses->scrap != current_scrap) {
                                         Scrap_Node *use = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                         use->scrap = current_scrap;
                                         use->next = name->uses;
                                         name->uses = use;
                                         add_uses(&(scrap_array(current_scrap).uses), name);
                                       }
#line 4452 "nuweb.w"
                            
                                       /* Fall through */
                                    case '|':
                                       sector = current_sector;
                                       /* Fall through */
                                    case '+':
                                       /* Add user identifier definition */
                                       
#line 4483 "nuweb.w"
                                       name = name_add(&user_names, new_name, sector);
                                       if (!name->defs || name->defs->scrap != current_scrap) {
                                         Scrap_Node *def = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                         def->scrap = current_scrap;
                                         def->next = name->defs;
                                         name->defs = def;
                                         add_uses(&(scrap_array(current_scrap).defs), name);
                                       }
#line 4458 "nuweb.w"
                            
                                       break;
                                    }
                                  }
                                } while (c != nw_char);
                                c = source_get();
                              }while (c == '|' || c == '*' || c == '-' || c == '+');
                              if (c != '}' && c != ']' && c != ')') {
                                fprintf(stderr, "%s: unexpected %c%c in index entry (%s, %d)\n",
                                        command_name, nw_char, c, source_name, source_line);
                                exit(-1);
                              }
                            }
#line 4372 "nuweb.w"
              
                            /* Fall through */
                  case ')':
                  case ']':
                  case '}': if (--depth > 0)
                              break;
                            /* else fall through */
                  case ',':
                            push('\0', &writer);
                            scrap_ended_with = c;
                            return current_scrap;
                  case '<': 
#line 4493 "nuweb.w"
                            {
                              Arglist * args = collect_scrap_name(current_scrap);
                              Name *name = args->name;
                              /* Save macro name */
                              
#line 4508 "nuweb.w"
                              {
                                char buff[24];
                              
                                push(nw_char, &writer);
                                push('<', &writer);
                                push(name->sector, &writer);
                                sprintf(buff, "%p", args);
                                pushs(buff, &writer);
                              }
#line 4496 "nuweb.w"
                            
                              add_to_use(name, current_scrap);
                              if (scrap_name_has_parameters) {
                                /* Save macro parameters */
                                
#line 1898 "nuweb.w"
                                {
                                  int param_scrap;
                                  char param_buf[10];
                                
                                  push(nw_char, &writer);
                                  push('(', &writer);
                                  do {
                                     param_scrap = collect_scrap();
                                     sprintf(param_buf, "%d", param_scrap);
                                     pushs(param_buf, &writer);
                                     push(nw_char, &writer);
                                     push(scrap_ended_with, &writer);
                                     add_to_use(name, current_scrap);
                                  } while( scrap_ended_with == ',' );
                                  do
                                    c = source_get();
                                  while( ' ' == c );
                                  if (c == nw_char) {
                                    c = source_get();
                                  }
                                  if (c != '>') {
                                    /* ZZZ print error */;
                                  }
                                }
#line 4499 "nuweb.w"
                            
                              }
                              push(nw_char, &writer);
                              push('>', &writer);
                              c = source_get();
                            }
#line 4383 "nuweb.w"
              
                            break;
                  case '%': 
#line 2780 "nuweb.w"
                            {
                                    do
                                            c = source_get();
                                    while (c != '\n');
                            }
#line 4385 "nuweb.w"
              
                            /* emit line break to the output file to keep #line in sync. */
                            push('\n', &writer);
                            c = source_get();
                            break;
                  case 'x': 
#line 4418 "nuweb.w"
                            {
                               /* Get label from */
                               
#line 6465 "nuweb.w"
                               char  label_name[MAX_NAME_LEN];
                               char * p = label_name;
                               while (c = 
#line 4419 "nuweb.w"
                                          source_get(), c != nw_char) /* Here is 149c-01 */
                                  *p++ = c;
                               *p = '\0';
                               c = 
#line 4419 "nuweb.w"
                                   source_get();
                               
#line 4419 "nuweb.w"
                            
                               /* Save label to label store */
                               
#line 6491 "nuweb.w"
                               if (label_name[0])
                               /* Search for label(<Complain about duplicate labels>,<Create a new label entry>) */

#line 6513 "nuweb.w"
                                                              {
                                  label_node * * plbl = &label_tab;
                                  for (;;)
                                  {
                                     label_node * lbl = *plbl;
                               
                                     if (lbl)
                                     {
                                        int cmp = label_name[0] - lbl->name[0];
                               
                                        if (cmp == 0)
                                           cmp = strcmp(label_name + 1, lbl->name + 1);
                                        if (cmp < 0)
                                           plbl = &lbl->left;
                                        else if (cmp > 0)
                                           plbl = &lbl->right;
                                        else
                                        {
                                           /* Complain about duplicate labels */
                                           
#line 6507 "nuweb.w"
                                           fprintf(stderr, "Duplicate label %s.\n", label_name);
                                           break;
                                        }
                                     }
                                     else
                                     {
                                         /* Create a new label entry */
                                         
#line 6499 "nuweb.w"
                                         lbl = (label_node *)arena_getmem(sizeof(label_node) + (p - label_name));
                                         lbl->left = lbl->right = NULL;
                                         strcpy(lbl->name, label_name);
                                         lbl->scrap = current_scrap;
                                         lbl->seq = ++lblseq;
                                         *plbl = lbl;
                                         break;
                                     }
                                  }
                               }
                               
#line 6492 "nuweb.w"
                               
                               else
                               {
                                  /* Complain about empty label */
                                  
#line 6510 "nuweb.w"
                                  fprintf(stderr, "Empty label.\n");
#line 6495 "nuweb.w"
                               
                               }
#line 4420 "nuweb.w"
                            
                               push(nw_char, &writer);
                               push('x', &writer);
                               pushs(label_name, &writer);
                               push(nw_char, &writer);
                            }
#line 4390 "nuweb.w"
              
                            break;
                  case 'c': 
#line 1729 "nuweb.w"
                            {
                               char * p = blockBuff;
                            
                               push(nw_char, &writer);
                               do
                               {
                                  push(c, &writer);
                                  c = *p++;
                               } while (c != '\0');
                            }
#line 4392 "nuweb.w"
              
                            break;
                  case '1': case '2': case '3':
                  case '4': case '5': case '6':
                  case '7': case '8': case '9':
                  case 'f': case '#': case 'v':
                  case 't': case 's':
                            push(nw_char, &writer);
                            break;
                  case '_': c = source_get();
                            break;
                  default :
                        if (c==nw_char)
                          {
                            push(nw_char, &writer);
                            push(nw_char, &writer);
                            c = source_get();
                            break;
                          }
                        fprintf(stderr, "%s: unexpected %c%c in scrap (%s, %d)\n",
                                    command_name, nw_char, c, source_name, source_line);
                            exit(-1);
                }
              }
#line 4351 "nuweb.w"
  
                  break;
            }
          push(c, &writer);
                  c = source_get();
                  break;
      }
    }
  }
#line 4316 "nuweb.w"

}

#line 4519 "nuweb.w"
void
add_to_use(Name * name, int current_scrap)
{
  if (!name->uses || name->uses->scrap != current_scrap) {
    Scrap_Node *use = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
    use->scrap = current_scrap;
    use->next = name->uses;
    name->uses = use;
  }
}

#line 4535 "nuweb.w"
static char pop(manager)
     Manager *manager;
{
  Slab *scrap = manager->scrap;
  int index = manager->index;
  char c = scrap->chars[index++];
  if (index == SLAB_SIZE) {
    manager->prev = scrap;
    manager->scrap = scrap->next;
    index = 0;
  }
  manager->index = index;
  return c;
}

#line 4552 "nuweb.w"
static void backup(n, manager)
     int n;
     Manager *manager;
{
  Slab *scrap = manager->scrap;
  int index = manager->index;
  if (n > index
      && manager->prev != NULL)
  {
     manager->scrap = manager->prev;
     manager->prev = NULL;
     index += SLAB_SIZE;
  }
  manager->index = (n <= index ? index - n : 0);
}

#line 4570 "nuweb.w"
void
lookup(int n, Arglist * par, char * arg[9], Name **name, Arglist ** args)
{
  int i;
  Arglist * p = par;

  for (i = 0; i < n && p != NULL; i++)
    p = p->next;
  if (p == NULL) {
    char * a = arg[n];

    *name = NULL;
    *args = (Arglist *)a;
  }
  else {
    *name = p->name;
    *args = p->args;
  }
}

#line 4592 "nuweb.w"
Arglist * instance(Arglist * a, Arglist * par, char * arg[9], int * ch)
{
   if (a != NULL) {
      int changed = 0;
      Arglist *args, *next;
      Name* name;
      /* Set up name, args and next */
      
#line 4614 "nuweb.w"
      next = instance(a->next, par, arg, &changed);
      name = a->name;
      if (name == (Name *)1) {
         Embed_Node * q = (Embed_Node *)arena_getmem(sizeof(Embed_Node));
         q->defs = (Scrap_Node *)a->args;
         q->args = par;
         args = (Arglist *)q;
         changed = 1;
      } else if (name != NULL)
        args = instance(a->args, par, arg, &changed);
      else {
        char * p = (char *)a->args;
         if (p[0] == ARG_CHR) {
            lookup(p[1] - '1', par, arg, &name, &args);
            changed = 1;
         }
         else {
            args = a->args;
         }
      }
#line 4598 "nuweb.w"

      if (changed){
        /* Build a new arglist */
        
#line 4636 "nuweb.w"
        a = (Arglist *)arena_getmem(sizeof(Arglist));
        a->name = name;
        a->args = args;
        a->next = next;
#line 4600 "nuweb.w"

        *ch = 1;
      }
   }

   return a;
}

#line 4642 "nuweb.w"
static Arglist *pop_scrap_name(manager, parameters)
     Manager *manager;
     Parameters *parameters;
{
  char name[MAX_NAME_LEN];
  char *p = name;
  int sector = pop(manager);
  int c = pop(manager);
  Arglist * args;

  while (c != nw_char) {
    *p++ = c;
    c = pop(manager);
  }
  *p = '\000';
  if (sscanf(name, "%p", &args) != 1)
  {
    fprintf(stderr,  "%s: found an internal problem (2)\n", command_name);
    exit(-1);
  }
  /* Check for end of scrap name */
  
#line 4669 "nuweb.w"
  {
    Name *pn;
    c = pop(manager);
    /* Check for macro parameters */
    
#line 1928 "nuweb.w"
    
      if (c == '(') {
        Parameters res = arena_getmem(10 * sizeof(int));
        int *p2 = res;
        int count = 0;
        int scrapnum;
    
        while( c && c != ')' ) {
          scrapnum = 0;
          c = pop(manager);
          while( '0' <= c && c <= '9' ) {
            scrapnum = scrapnum  * 10 + c - '0';
            c = pop(manager);
          }
          if ( c == nw_char ) {
            c = pop(manager);
          }
          *p2++ = scrapnum;
        }
        while (count < 10) {
          *p2++ = 0;
          count++;
        }
        while( c && c != nw_char ) {
            c = pop(manager);
        }
        if ( c == nw_char ) {
          c = pop(manager);
        }
        *parameters = res;
      }
    
#line 4672 "nuweb.w"
  
  }
#line 4662 "nuweb.w"

  return args;
}

#line 4676 "nuweb.w"
int write_scraps(file, spelling, defs, global_indent, indent_chars,
                   debug_flag, tab_flag, indent_flag,
                   comment_flag, inArgs, inParams, parameters, title)
     FILE *file;
     char * spelling;
     Scrap_Node *defs;
     int global_indent;
     char *indent_chars;
     char debug_flag;
     char tab_flag;
     char indent_flag;
     unsigned char comment_flag;
     Arglist * inArgs;
     char * inParams[9];
     Parameters parameters;
     char * title;
{
  /* This is in file scraps.c */
  int indent = 0;
  int newline = 1;
  int iter = 0;
  while (defs) {
    /* Copy \verb|defs->scrap| to \verb|file| */
    
#line 4710 "nuweb.w"
    {
      char c;
      Manager reader;
      Parameters local_parameters = 0;
      int line_number = scrap_array(defs->scrap).file_line;
      reader.scrap = scrap_array(defs->scrap).slab;
      reader.index = 0;
      /* Insert debugging information if required */
      
#line 4774 "nuweb.w"
      if (debug_flag 
        //&& 
        //source_last != '['
        //scrap_type != 1
          //0
      ) {
        fprintf(file, "\n#line %d \"%s\"\n",
                line_number, scrap_array(defs->scrap).file_name);
        /* Insert appropriate indentation */
        
#line 4787 "nuweb.w"
        {
          char c1 = pop(&reader);
          char c2 = pop(&reader);
        
          if (indent_flag && !(
#line 4813 "nuweb.w"
                               (c1 == '\n'
                               || c1 == nw_char) && (c2 == '#' || (delayed_indent |= (c2 == '<')))
#line 4791 "nuweb.w"
        )) {
            /* Put out the indent */
            
#line 4799 "nuweb.w"
            if (tab_flag)
                for (indent=0; indent<global_indent; indent++)
                  putc(' ', file);
              else
                for (indent=0; indent<global_indent; indent++)
                  putc(indent_chars[indent], file);
            
#line 4792 "nuweb.w"
        
          }
          indent = 0;
          backup(2, &reader);
        }
#line 4782 "nuweb.w"
      
      }
#line 4717 "nuweb.w"
    
      if (delayed_indent)
      {
        /* Insert appropriate indentation */
        
#line 4787 "nuweb.w"
        {
          char c1 = pop(&reader);
          char c2 = pop(&reader);
        
          if (indent_flag && !(
#line 4813 "nuweb.w"
                               (c1 == '\n'
                               || c1 == nw_char) && (c2 == '#' || (delayed_indent |= (c2 == '<')))
#line 4791 "nuweb.w"
        )) {
            /* Put out the indent */
            
#line 4799 "nuweb.w"
            if (tab_flag)
                for (indent=0; indent<global_indent; indent++)
                  putc(' ', file);
              else
                for (indent=0; indent<global_indent; indent++)
                  putc(indent_chars[indent], file);
            
#line 4792 "nuweb.w"
        
          }
          indent = 0;
          backup(2, &reader);
        }
#line 4720 "nuweb.w"
    
      }
      c = pop(&reader);
      while (c) {
        switch (c) {
          case '\n':
             if (global_indent >= 0) {
               putc(c, file);
               line_number++;
               newline = 1;
               delayed_indent = 0;
               /* Insert appropriate indentation */
               
#line 4787 "nuweb.w"
               {
                 char c1 = pop(&reader);
                 char c2 = pop(&reader);
               
                 if (indent_flag && !(
#line 4813 "nuweb.w"
                                      (c1 == '\n'
                                      || c1 == nw_char) && (c2 == '#' || (delayed_indent |= (c2 == '<')))
#line 4791 "nuweb.w"
               )) {
                   /* Put out the indent */
                   
#line 4799 "nuweb.w"
                   if (tab_flag)
                       for (indent=0; indent<global_indent; indent++)
                         putc(' ', file);
                     else
                       for (indent=0; indent<global_indent; indent++)
                         putc(indent_chars[indent], file);
                   
#line 4792 "nuweb.w"
               
                 }
                 indent = 0;
                 backup(2, &reader);
               }
#line 4731 "nuweb.w"
    
               break;
             } else {
               /* Don't show newlines in embedded fragmants */
               fputs(". . .", file);
               return indent + global_indent;
             }
          case '\t': 
#line 4817 "nuweb.w"
                     {
                       if (tab_flag)
                         /* Expand tab into spaces */
                         
#line 2691 "nuweb.w"
                         {
                           int delta = 8 - (indent % 8);
                           indent += delta;
                           while (delta > 0) {
                             putc(' ', file);
                             delta--;
                           }
                         }
#line 4819 "nuweb.w"
                     
                       else {
                         putc('\t', file);
                         if (global_indent >= 0) {
                           /* Add more indentation ''\t'' */
                           
#line 4762 "nuweb.w"
                           {
                             if (global_indent + indent >= MAX_INDENT) {
                               fprintf(stderr,
                                      "Error! maximum indentation exceeded in \"%s\".\n",
                                       spelling);
                               exit(1);
                             }
                             indent_chars[global_indent + indent] = '\t';
                           }
#line 4823 "nuweb.w"
                     
                         }
                         indent++;
                       }
                     }
#line 4738 "nuweb.w"
    
                     delayed_indent = 0;
                     break;
          default:
             if (c==nw_char)
               {
                 /* Check for macro invocation in scrap */
                 
#line 4831 "nuweb.w"
                 {
                   int oldin = indent;
                   char oldcf = comment_flag;
                   c = pop(&reader);
                   switch (c) {
                     case 't': 
#line 4955 "nuweb.w"
                               {
                                  char * p = title;
                                  Arglist *q = inArgs;
                                  int narg;
                               
                                  /* Comment this macro use */
                                  
#line 4968 "nuweb.w"
                                  narg = 0;
                                  while (*p != '\000') {
                                    if (*p == ARG_CHR) {
                                      if (q == NULL) {
                                         if (defs->quoted)
                                            fprintf(file, "%c'%s%c'", nw_char, inParams[narg], nw_char);
                                         else
                                            fprintf(file, "'%s'", inParams[narg]);
                                      }
                                      else {
                                        comment_ArglistElement(file, q, defs->quoted);
                                        q = q->next;
                                      }
                                      p++;
                                      narg++;
                                    }
                                    else
                                       fputc(*p++, file);
                                  }
#line 4960 "nuweb.w"
                               
                                  if (xref_flag) {
                                     putc(' ', file);
                                     write_single_scrap_ref(file, defs->scrap);
                                  }
                               }
#line 4836 "nuweb.w"
                 
                               break;
                     case 'c': 
#line 1741 "nuweb.w"
                               {
                                  int bgn = indent + global_indent;
                                  int posn = bgn + strlen(comment_begin[comment_flag]);
                                  int i;
                               
                                  /* Perhaps put a delayed indent */
                                  
#line 4949 "nuweb.w"
                                  if (delayed_indent)
                                     for (i = indent + global_indent; --i >= 0; )
                                        putc(' ', file);
                                  
#line 1746 "nuweb.w"
                               
                                  c = pop(&reader);
                                  fputs(comment_begin[comment_flag], file);
                                  while (c != '\0')
                                  {
                                     /* Move a word to the file */
                                     
#line 1770 "nuweb.w"
                                     do
                                     {
                                        putc(c, file);
                                        posn += 1;
                                        c = pop(&reader);
                                     } while (c > ' ');
                                     
#line 1751 "nuweb.w"
                               
                                     /* If we break the line at this word */
                                     
#line 1779 "nuweb.w"
                                     if (c == '\n' || (c == ' ' && posn > 60))
#line 1752 "nuweb.w"
                               
                                     {
                                        putc('\n', file);
                                        for (i = 0; i < bgn ; i++)
                                           putc(' ', file);
                                        c = pop(&reader);
                                        if (c != '\0')
                                        {
                                           posn = bgn + strlen(comment_mid[comment_flag]);
                                           fputs(comment_mid[comment_flag], file);
                                        }
                                     }
                                  }
                                  fputs(comment_end[comment_flag], file);
                               }
                               
#line 4838 "nuweb.w"
                 
                               break;
                     case 'f': 
#line 4879 "nuweb.w"
                               if (defs->quoted)
                                  fprintf(file, "%cf", nw_char);
                               else
                                  fputs(spelling, file);
                               
#line 4840 "nuweb.w"
                 
                               break;
                     case 'x': 
#line 4873 "nuweb.w"
                               {
                                  /* Get label from */
                                  
#line 6465 "nuweb.w"
                                  char  label_name[MAX_NAME_LEN];
                                  char * p = label_name;
                                  while (c = 
#line 4874 "nuweb.w"
                                             pop(&reader), c != nw_char) /* Here is 149c-01 */
                                     *p++ = c;
                                  *p = '\0';
                                  c = 
#line 4874 "nuweb.w"
                                      pop(&reader);
                                  
#line 4874 "nuweb.w"
                               
                                  write_label(label_name, file);
                               }
#line 4842 "nuweb.w"
                 
                     case '_': break;
                     case 'v': 
#line 2757 "nuweb.w"
                               fputs(version_string, file);
                               
#line 4844 "nuweb.w"
                 
                               break;
                     case 's': indent = -global_indent;
                               comment_flag = 0;
                               break;
                     case '<': 
#line 4886 "nuweb.w"
                               {
                                 Arglist *a = pop_scrap_name(&reader, &local_parameters);
                                 Name *name = a->name;
                                 int changed;
                                 Arglist * args = instance(a->args, inArgs, inParams, &changed);
                                 int i, narg;
                                 char * p = name->spelling;
                                 char * * inParams = name->arg;
                                 Arglist *q = args;
                               
                                 if (name->mark) {
                                   fprintf(stderr, "%s: recursive macro discovered involving <%s>\n",
                                           command_name, name->spelling);
                                   exit(-1);
                                 }
                                 if (name->defs && !defs->quoted) {
                                   /* Perhaps comment this macro */
                                   
#line 4931 "nuweb.w"
                                   if (comment_flag && newline) {
                                      /* Perhaps put a delayed indent */
                                      
#line 4949 "nuweb.w"
                                      if (delayed_indent)
                                         for (i = indent + global_indent; --i >= 0; )
                                            putc(' ', file);
                                      
#line 4932 "nuweb.w"
                                   
                                      fputs(comment_begin[comment_flag], file);
                                      /* Comment this macro use */
                                      
#line 4968 "nuweb.w"
                                      narg = 0;
                                      while (*p != '\000') {
                                        if (*p == ARG_CHR) {
                                          if (q == NULL) {
                                             if (defs->quoted)
                                                fprintf(file, "%c'%s%c'", nw_char, inParams[narg], nw_char);
                                             else
                                                fprintf(file, "'%s'", inParams[narg]);
                                          }
                                          else {
                                            comment_ArglistElement(file, q, defs->quoted);
                                            q = q->next;
                                          }
                                          p++;
                                          narg++;
                                        }
                                        else
                                           fputc(*p++, file);
                                      }
#line 4934 "nuweb.w"
                                   
                                      if (xref_flag) {
                                         putc(' ', file);
                                         write_single_scrap_ref(file, name->defs->scrap);
                                      }
                                      if (comment_end)
                                         fputs(comment_end[comment_flag], file);
                                      putc('\n', file);
                                      if (!delayed_indent)
                                         for (i = indent + global_indent; --i >= 0; )
                                            putc(' ', file);
                                   }
                                   
#line 4902 "nuweb.w"
                               
                                   name->mark = TRUE;
                                   indent = write_scraps(file, spelling, name->defs, global_indent + indent,
                                                         indent_chars, debug_flag, tab_flag, indent_flag,
                                                         comment_flag, args, name->arg,
                                                         local_parameters, name->spelling);
                                   indent -= global_indent;
                                   name->mark = FALSE;
                                 }
                                 else
                                 {
                                   if (delayed_indent)
                                   {
                                     for (i = indent + global_indent; --i >= 0; )
                                        putc(' ', file);
                                   }
                               
                                   fprintf(file, "%c<",  nw_char);
                                   if (name->sector == 0)
                                      fputc('+', file);
                                   /* Comment this macro use */
                                   
#line 4968 "nuweb.w"
                                   narg = 0;
                                   while (*p != '\000') {
                                     if (*p == ARG_CHR) {
                                       if (q == NULL) {
                                          if (defs->quoted)
                                             fprintf(file, "%c'%s%c'", nw_char, inParams[narg], nw_char);
                                          else
                                             fprintf(file, "'%s'", inParams[narg]);
                                       }
                                       else {
                                         comment_ArglistElement(file, q, defs->quoted);
                                         q = q->next;
                                       }
                                       p++;
                                       narg++;
                                     }
                                     else
                                        fputc(*p++, file);
                                   }
#line 4922 "nuweb.w"
                               
                                   fprintf(file, "%c>",  nw_char);
                                   if (!defs->quoted && !tex_flag)
                                     fprintf(stderr, "%s: macro never defined <%s>\n",
                                           command_name, name->spelling);
                                 }
                               }
#line 4849 "nuweb.w"
                 
                               /* Insert debugging information if required */
                               
#line 4774 "nuweb.w"
                               if (debug_flag 
                                 //&& 
                                 //source_last != '['
                                 //scrap_type != 1
                                   //0
                               ) {
                                 fprintf(file, "\n#line %d \"%s\"\n",
                                         line_number, scrap_array(defs->scrap).file_name);
                                 /* Insert appropriate indentation */
                                 
#line 4787 "nuweb.w"
                                 {
                                   char c1 = pop(&reader);
                                   char c2 = pop(&reader);
                                 
                                   if (indent_flag && !(
#line 4813 "nuweb.w"
                                                        (c1 == '\n'
                                                        || c1 == nw_char) && (c2 == '#' || (delayed_indent |= (c2 == '<')))
#line 4791 "nuweb.w"
                                 )) {
                                     /* Put out the indent */
                                     
#line 4799 "nuweb.w"
                                     if (tab_flag)
                                         for (indent=0; indent<global_indent; indent++)
                                           putc(' ', file);
                                       else
                                         for (indent=0; indent<global_indent; indent++)
                                           putc(indent_chars[indent], file);
                                     
#line 4792 "nuweb.w"
                                 
                                   }
                                   indent = 0;
                                   backup(2, &reader);
                                 }
#line 4782 "nuweb.w"
                               
                               }
#line 4850 "nuweb.w"
                 
                               indent = oldin;
                               comment_flag = oldcf;
                               break;
                     /* Handle macro parameter substitution */
                     
#line 1828 "nuweb.w"
                     
                     case '1': case '2': case '3':
                     case '4': case '5': case '6':
                     case '7': case '8': case '9':
                       {
                         Arglist * args;
                         Name * name;
                     
                         lookup(c - '1', inArgs, inParams, &name, &args);
                     
                         if (name == (Name *)1) {
                           Embed_Node * q = (Embed_Node *)args;
                           indent = write_scraps(file, spelling, q->defs,
                                                 global_indent + indent,
                                                 indent_chars, debug_flag,
                                                 tab_flag, indent_flag,
                                                 0,
                                                 q->args, inParams,
                                                 local_parameters, "");
                         }
                         else if (name != NULL) {
                            int i, narg;
                            char * p = name->spelling;
                            Arglist *q = args;
                     
                            /* Perhaps comment this macro */
                            
#line 4931 "nuweb.w"
                            if (comment_flag && newline) {
                               /* Perhaps put a delayed indent */
                               
#line 4949 "nuweb.w"
                               if (delayed_indent)
                                  for (i = indent + global_indent; --i >= 0; )
                                     putc(' ', file);
                               
#line 4932 "nuweb.w"
                            
                               fputs(comment_begin[comment_flag], file);
                               /* Comment this macro use */
                               
#line 4968 "nuweb.w"
                               narg = 0;
                               while (*p != '\000') {
                                 if (*p == ARG_CHR) {
                                   if (q == NULL) {
                                      if (defs->quoted)
                                         fprintf(file, "%c'%s%c'", nw_char, inParams[narg], nw_char);
                                      else
                                         fprintf(file, "'%s'", inParams[narg]);
                                   }
                                   else {
                                     comment_ArglistElement(file, q, defs->quoted);
                                     q = q->next;
                                   }
                                   p++;
                                   narg++;
                                 }
                                 else
                                    fputc(*p++, file);
                               }
#line 4934 "nuweb.w"
                            
                               if (xref_flag) {
                                  putc(' ', file);
                                  write_single_scrap_ref(file, name->defs->scrap);
                               }
                               if (comment_end)
                                  fputs(comment_end[comment_flag], file);
                               putc('\n', file);
                               if (!delayed_indent)
                                  for (i = indent + global_indent; --i >= 0; )
                                     putc(' ', file);
                            }
                            
#line 1853 "nuweb.w"
                     
                            indent = write_scraps(file, spelling, name->defs,
                                                  global_indent + indent,
                                                  indent_chars, debug_flag,
                                                  tab_flag, indent_flag,
                                                  comment_flag, args, name->arg,
                                                  local_parameters, p);
                         }
                         else if (args != NULL) {
                            if (delayed_indent) {
                              /* Put out the indent */
                              
#line 4799 "nuweb.w"
                              if (tab_flag)
                                  for (indent=0; indent<global_indent; indent++)
                                    putc(' ', file);
                                else
                                  for (indent=0; indent<global_indent; indent++)
                                    putc(indent_chars[indent], file);
                              
#line 1863 "nuweb.w"
                     
                            }
                            fputs((char *)args, file);
                         }
                         else if ( parameters && parameters[c - '1'] ) {
                           Scrap_Node param_defs;
                           param_defs.scrap = parameters[c - '1'];
                           param_defs.next = 0;
                           write_scraps(file, spelling, &param_defs,
                                        global_indent + indent,
                                        indent_chars, debug_flag,
                                        tab_flag, indent_flag,
                                        comment_flag, NULL, NULL, 0, "");
                         } else if (delayed_indent) {
                           /* Put out the indent */
                           
#line 4799 "nuweb.w"
                           if (tab_flag)
                               for (indent=0; indent<global_indent; indent++)
                                 putc(' ', file);
                             else
                               for (indent=0; indent<global_indent; indent++)
                                 putc(indent_chars[indent], file);
                           
#line 1877 "nuweb.w"
                     
                         }
                       }
                     
#line 4854 "nuweb.w"
                 
                               indent = oldin;
                               break;
                     default:
                           if(c==nw_char)
                             {
                               putc(c, file);
                               if (global_indent >= 0) {
                                  /* Add more indentation '' '' */
                                  
#line 4762 "nuweb.w"
                                  {
                                    if (global_indent + indent >= MAX_INDENT) {
                                      fprintf(stderr,
                                             "Error! maximum indentation exceeded in \"%s\".\n",
                                              spelling);
                                      exit(1);
                                    }
                                    indent_chars[global_indent + indent] = ' ';
                                  }
#line 4862 "nuweb.w"
                 
                               }
                               indent++;
                               break;
                             }
                           /* ignore, since we should already have a warning */
                               break;
                   }
                 }
#line 4744 "nuweb.w"
    
                 break;
               }
             putc(c, file);
             if (global_indent >= 0) {
               /* Add more indentation '' '' */
               
#line 4762 "nuweb.w"
               {
                 if (global_indent + indent >= MAX_INDENT) {
                   fprintf(stderr,
                          "Error! maximum indentation exceeded in \"%s\".\n",
                           spelling);
                   exit(1);
                 }
                 indent_chars[global_indent + indent] = ' ';
               }
#line 4749 "nuweb.w"
    
             }
             indent++;
             if (c > ' ') newline = 0;
             delayed_indent = 0;
             break;
        }
        c = pop(&reader);
      }
    }
#line 4698 "nuweb.w"

    defs = defs->next;
  }
  return indent + global_indent;
}

#line 5041 "nuweb.w"
void collect_numbers(aux_name)
     char *aux_name;
{
  if (number_flag) {
    int i;
    for (i=1; i<scraps; i++)
      scrap_array(i).page = i;
  }
  else {
    FILE *aux_file = fopen(aux_name, "r");
    already_warned = FALSE;
    if (aux_file) {
      char aux_line[500];
      while (fgets(aux_line, 500, aux_file)) {
        /* Read line in \verb|.aux| file */
        
#line 5068 "nuweb.w"
        
        int scrap_number;
        int page_number;
        int i;
        int dummy_idx;
        int bracket_depth = 1;
        if (1 == sscanf(aux_line,
                        "\\newlabel{scrap%d}{{%n",
                        &scrap_number,
                        &dummy_idx)) {
          for (i = dummy_idx; i < strlen(aux_line) && bracket_depth > 0; i++) {
            if (aux_line[i] == '{') bracket_depth++;
            else if (aux_line[i] == '}') bracket_depth--;
          }
          if (i > dummy_idx
              && i < strlen(aux_line)
              && 1 == sscanf(aux_line+i, "{%d}" ,&page_number)) {
            if (scrap_number < scraps)
              scrap_array(scrap_number).page = page_number;
            else
              /* Warn (only once) about needing to rerun after Latex */
              
#line 4255 "nuweb.w"
              {
                if (!already_warned) {
                  fprintf(stderr, "%s: you'll need to rerun nuweb after running latex\n",
                          command_name);
                  already_warned = TRUE;
                }
              }
#line 5088 "nuweb.w"
        
          }
        }
        
#line 5055 "nuweb.w"

      }
      fclose(aux_file);
      /* Add letters to scraps with duplicate page numbers */
      
#line 5095 "nuweb.w"
      {
         int i = 0;
      
         /* Step 'i' to the next valid scrap */
         
#line 5109 "nuweb.w"
         do
            i++;
         while (i < scraps && scrap_array(i).page == -1);
         
#line 5098 "nuweb.w"
      
         /* For all remaining scraps */
         
#line 5115 "nuweb.w"
         while (i < scraps)
#line 5099 "nuweb.w"
       {
            int j = i;
            /* Step 'j' to the next valid scrap */
            
#line 5109 "nuweb.w"
            do
               j++;
            while (j < scraps && scrap_array(j).page == -1);
            
#line 5101 "nuweb.w"
      
            /* Perhaps add letters to the page numbers */
            
#line 5118 "nuweb.w"
            if (scrap_array(i).page == scrap_array(j).page) {
               if (scrap_array(i).letter == 0)
                  scrap_array(i).letter = 'a';
               scrap_array(j).letter = scrap_array(i).letter + 1;
            }
            
#line 5102 "nuweb.w"
      
            i = j;
         }
      }
      
#line 5058 "nuweb.w"

    }
  }
}

#line 5890 "nuweb.w"
typedef struct name_node {
  struct name_node *next;
  Name *name;
} Name_Node;

#line 5897 "nuweb.w"
typedef struct goto_node {
  Name_Node *output;            /* list of words ending in this state */
  struct move_node *moves;      /* list of possible moves */
  struct goto_node *fail;       /* and where to go when no move fits */
  struct goto_node *next;       /* next goto node with same depth */
} Goto_Node;

#line 5906 "nuweb.w"
typedef struct move_node {
  struct move_node *next;
  Goto_Node *state;
  char c;
} Move_Node;

#line 5914 "nuweb.w"
static Goto_Node *root[128];
static int max_depth;
static Goto_Node **depths;

#line 5921 "nuweb.w"
static Goto_Node *goto_lookup(c, g)
     char c;
     Goto_Node *g;
{
  Move_Node *m = g->moves;
  while (m && m->c != c)
    m = m->next;
  if (m)
    return m->state;
  else
    return NULL;
}

#line 5938 "nuweb.w"
typedef struct ArgMgr_s
{
   char * pv;
   char * bgn;
   Arglist * arg;
   struct ArgMgr_s * old;
} ArgMgr;

#line 5948 "nuweb.w"
typedef struct ArgManager_s
{
   Manager * m;
   ArgMgr * a;
} ArgManager;

#line 5956 "nuweb.w"
static void
pushArglist(ArgManager * mgr, Arglist * a)
{
   ArgMgr * b = malloc(sizeof(ArgMgr));

   if (b == NULL)
   {
      fprintf(stderr, "Can't allocate space for an argument manager\n");
      exit(EXIT_FAILURE);
   }
   b->pv = b->bgn = NULL;
   b->arg = a;
   b->old = mgr->a;
   mgr->a = b;
}

#line 5974 "nuweb.w"
static char argpop(ArgManager * mgr)
{
   while (mgr->a != NULL)
   {
      ArgMgr * a = mgr->a;

      /* Perhaps |return| a character from the current arg */
      
#line 5992 "nuweb.w"
      if (a->pv != NULL)
      {
         char c = *a->pv++;
      
         if (c != '\0')
            return c;
         a->pv = NULL;
         return ' ';
      }
      
#line 5980 "nuweb.w"

      /* Perhaps start a new arg */
      
#line 6008 "nuweb.w"
      if (a->arg) {
         Arglist * b = a->arg;
      
         a->arg = b->next;
         if (b->name == NULL) {
            a->bgn = a->pv = (char *)b->args;
         } else if (b->name == (Name *)1) {
            a->bgn = a->pv = "{Embedded Scrap}";
         } else {
            pushArglist(mgr, b->args);
         }
#line 5981 "nuweb.w"

      /* Otherwise pop the current arg */
      
#line 6021 "nuweb.w"
      } else {
         mgr->a = a->old;
         free(a);
      }
#line 5982 "nuweb.w"

   }

   return (pop(mgr->m));
}

#line 6027 "nuweb.w"
static char
prev_char(ArgManager * mgr, int n)
{
   char c = '\0';
   ArgMgr * a = mgr->a;
   Manager * m = mgr->m;

   if (a != NULL) {
      /* Get the nth previous character from an argument */
      
#line 6045 "nuweb.w"
      if (a->pv && a->pv - n >= a->bgn)
         c = *a->pv;
      else if (a->bgn) {
         int j = strlen(a->bgn) + 1;
      
         if (n >= j)
            c = a->bgn[j - n];
         else
            c = ' ';
      }
      
#line 6035 "nuweb.w"

   } else {
      /* Get the nth previous character from a scrap */
      
#line 6058 "nuweb.w"
      int k = m->index - n - 2;
      
      if (k >= 0)
         c = m->scrap->chars[k];
      else if (m->prev)
         c = m->prev->chars[SLAB_SIZE - k];
      
#line 6037 "nuweb.w"

   }

   return c;
}

#line 6073 "nuweb.w"
static void build_gotos();
static int reject_match();

void search()
{
  int i;
  for (i=0; i<128; i++)
    root[i] = NULL;
  max_depth = 10;
  depths = (Goto_Node **) arena_getmem(max_depth * sizeof(Goto_Node *));
  for (i=0; i<max_depth; i++)
    depths[i] = NULL;
  build_gotos(user_names);
  /* Build failure functions */
  
#line 6160 "nuweb.w"
  {
    int depth;
    for (depth=1; depth<max_depth; depth++) {
      Goto_Node *r = depths[depth];
      while (r) {
        Move_Node *m = r->moves;
        while (m) {
          char a = m->c;
          Goto_Node *s = m->state;
          Goto_Node *state = r->fail;
          while (state && !goto_lookup(a, state))
            state = state->fail;
          if (state)
            s->fail = goto_lookup(a, state);
          else
            s->fail = root[a];
          if (s->fail) {
            Name_Node *p = s->fail->output;
            while (p) {
              Name_Node *q = (Name_Node *) arena_getmem(sizeof(Name_Node));
              q->name = p->name;
              q->next = s->output;
              s->output = q;
              p = p->next;
            }
          }
          m = m->next;
        }
        r = r->next;
      }
    }
  }
#line 6086 "nuweb.w"

  /* Search scraps */
  
#line 6197 "nuweb.w"
  {
    for (i=1; i<scraps; i++) {
      char c, last = '\0';
      Manager rd;
      ArgManager reader;
      Goto_Node *state = NULL;
      rd.prev = NULL;
      rd.scrap = scrap_array(i).slab;
      rd.index = 0;
      reader.m = &rd;
      reader.a = NULL;
      c = argpop(&reader);
      while (c) {
        while (state && !goto_lookup(c, state))
          state = state->fail;
        if (state)
          state = goto_lookup(c, state);
        else
          state = root[c];
        /* Skip over at at */
        
#line 6244 "nuweb.w"
        if (last == nw_char && c == nw_char)
        {
           last = '\0';
           c = argpop(&reader);
        }
        
#line 6216 "nuweb.w"
  
        /* Skip over a scrap use */
        
#line 6252 "nuweb.w"
        if (last == nw_char && c == '<')
        {
           char buf[MAX_NAME_LEN];
           char * p = buf;
           Arglist * args;
        
           c = argpop(&reader);
           while ((c = argpop(&reader)) != nw_char)
              *p++ = c;
           c = argpop(&reader);
           *p = '\0';
           if (sscanf(buf, "%p", &args) != 1) {
              fprintf(stderr,  "%s: found an internal problem (3)\n", command_name);
              exit(-1);
           }
           pushArglist(&reader, args);
        }
#line 6217 "nuweb.w"
  
        /* Skip over a block comment */
        
#line 1782 "nuweb.w"
        if (last == nw_char && c == 'c')
           while ((c = pop(reader.m)) != '\0')
              /* Skip */;
        
#line 6218 "nuweb.w"
  
        last = c;
        c = argpop(&reader);
        if (state && state->output) {
          Name_Node *p = state->output;
          do {
            Name *name = p->name;
            if (!reject_match(name, c, &reader) &&
                scrap_array(i).sector == name->sector &&
                (!name->uses || name->uses->scrap != i)) {
              Scrap_Node *new_use =
                  (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
              new_use->scrap = i;
              new_use->next = name->uses;
              name->uses = new_use;
              if (!scrap_is_in(name->defs, i))
                add_uses(&(scrap_array(i).uses), name);
            }
            p = p->next;
          } while (p);
        }
      }
    }
  }
#line 6087 "nuweb.w"

}

#line 6094 "nuweb.w"
static void build_gotos(tree)
     Name *tree;
{
  while (tree) {
    /* Extend goto graph with \verb|tree->spelling| */
    
#line 6106 "nuweb.w"
    {
      int depth = 2;
      char *p = tree->spelling;
      char c = *p++;
      Goto_Node *q = root[c];
      Name_Node * last;
      if (!q) {
        q = (Goto_Node *) arena_getmem(sizeof(Goto_Node));
        root[c] = q;
        q->moves = NULL;
        q->fail = NULL;
        q->moves = NULL;
        q->output = NULL;
        q->next = depths[1];
        depths[1] = q;
      }
      while ((c = *p++)) {
        Goto_Node *new = goto_lookup(c, q);
        if (!new) {
          Move_Node *new_move = (Move_Node *) arena_getmem(sizeof(Move_Node));
          new = (Goto_Node *) arena_getmem(sizeof(Goto_Node));
          new->moves = NULL;
          new->fail = NULL;
          new->moves = NULL;
          new->output = NULL;
          new_move->state = new;
          new_move->c = c;
          new_move->next = q->moves;
          q->moves = new_move;
          if (depth == max_depth) {
            int i;
            Goto_Node **new_depths =
                (Goto_Node **) arena_getmem(2*depth*sizeof(Goto_Node *));
            max_depth = 2 * depth;
            for (i=0; i<depth; i++)
              new_depths[i] = depths[i];
            depths = new_depths;
            for (i=depth; i<max_depth; i++)
              depths[i] = NULL;
          }
          new->next = depths[depth];
          depths[depth] = new;
        }
        q = new;
        depth++;
      }
      last = q->output;
      q->output = (Name_Node *) arena_getmem(sizeof(Name_Node));
      q->output->next = last;
      q->output->name = tree;
    }
#line 6098 "nuweb.w"

    build_gotos(tree->rlink);
    tree = tree->llink;
  }
}

#line 6277 "nuweb.w"

static int scrap_is_in(Scrap_Node * list, int i)
{
  while (list != NULL) {
    if (list->scrap == i)
      return TRUE;
    list = list->next;
  }
  return FALSE;
}

#line 6290 "nuweb.w"

static void add_uses(Uses * * root, Name *name)
{
   int cmp;
   int robs_strcmp(char *, char *);
   Uses *p, **q = root;

   while ((p = *q, p != NULL)
          && (cmp = robs_strcmp(p->defn->spelling, name->spelling)) < 0)
      q = &(p->next);
   if (p == NULL || cmp > 0)
   {
      Uses *new = arena_getmem(sizeof(Uses));
      new->next = p;
      new->defn = name;
      *q = new;
   }
}

#line 6318 "nuweb.w"

void
format_uses_refs(FILE * tex_file, int scrap)
{
  Uses * p = scrap_array(scrap).uses;
  if (p != NULL)
    /* Write uses references */
    
#line 6329 "nuweb.w"
    {
      char join = ' ';
      fputs("\\item \\NWtxtIdentsUsed\\nobreak\\", tex_file);
      do {
        /* Write one use reference */
        
#line 6341 "nuweb.w"
        Name * name = p->defn;
        Scrap_Node *defs = name->defs;
        int first = TRUE, page = -1;
        fprintf(tex_file,
                "%c \\verb%c%s%c\\nobreak\\ ",
                join, nw_char, name->spelling, nw_char);
        if (defs)
        {
          do {
            /* Write one referenced scrap */
            
#line 6362 "nuweb.w"
            fputs("\\NWlink{nuweb", tex_file);
            write_scrap_ref(tex_file, defs->scrap, -1, &page);
            fputs("}{", tex_file);
            write_scrap_ref(tex_file, defs->scrap, first, &page);
            fputs("}", tex_file);
#line 6350 "nuweb.w"
        
            first = FALSE;
            defs = defs->next;
          }while (defs!= NULL);
        }
        else
        {
          fputs("\\NWnotglobal", tex_file);
        }
        
#line 6333 "nuweb.w"
    
        join = ',';
        p = p->next;
      }while (p != NULL);
      fputs(".", tex_file);
    }
#line 6324 "nuweb.w"

}

#line 6369 "nuweb.w"

void
format_defs_refs(FILE * tex_file, int scrap)
{
  Uses * p = scrap_array(scrap).defs;
  if (p != NULL)
    /* Write defs references */
    
#line 6380 "nuweb.w"
    {
      char join = ' ';
      fputs("\\item \\NWtxtIdentsDefed\\nobreak\\", tex_file);
      do {
        /* Write one def reference */
        
#line 6392 "nuweb.w"
        Name * name = p->defn;
        Scrap_Node *defs = name->uses;
        int first = TRUE, page = -1;
        fprintf(tex_file,
                "%c \\verb%c%s%c\\nobreak\\ ",
                join, nw_char, name->spelling, nw_char);
        if (defs == NULL
            || (defs->scrap == scrap && defs->next == NULL)) {
          fputs("\\NWtxtIdentsNotUsed", tex_file);
        }
        else {
          do {
            if (defs->scrap != scrap) {
               /* Write one referenced scrap */
               
#line 6362 "nuweb.w"
               fputs("\\NWlink{nuweb", tex_file);
               write_scrap_ref(tex_file, defs->scrap, -1, &page);
               fputs("}{", tex_file);
               write_scrap_ref(tex_file, defs->scrap, first, &page);
               fputs("}", tex_file);
#line 6405 "nuweb.w"
        
               first = FALSE;
            }
            defs = defs->next;
          }while (defs!= NULL);
        }
        
#line 6384 "nuweb.w"
    
        join = ',';
        p = p->next;
      }while (p != NULL);
      fputs(".", tex_file);
    }
#line 6375 "nuweb.w"

}

#line 6424 "nuweb.w"
#define sym_char(c) (isalnum(c) || (c) == '_')

static int op_char(c)
     char c;
{
  switch (c) {
    case '!':           case '#': case '%': case '$': case '^':
    case '&': case '*': case '-': case '+': case '=': case '/':
    case '|': case '~': case '<': case '>':
      return TRUE;
    default:
      return c==nw_char ? TRUE : FALSE;
  }
}

#line 6441 "nuweb.w"
static int reject_match(name, post, reader)
     Name *name;
     char post;
     ArgManager *reader;
{
  int len = strlen(name->spelling);
  char first = name->spelling[0];
  char last = name->spelling[len - 1];
  char prev = prev_char(reader, len);
  if (sym_char(last) && sym_char(post)) return TRUE;
  if (sym_char(first) && sym_char(prev)) return TRUE;
  if (op_char(last) && op_char(post)) return TRUE;
  if (op_char(first) && op_char(prev)) return TRUE;
  return FALSE; /* Here is 149b-01 */
}

#line 6474 "nuweb.w"
void
write_label(char label_name[], FILE * file)
/* Search for label(<Write the label to file>,<Complain about missing label>) */

#line 6513 "nuweb.w"
{
   label_node * * plbl = &label_tab;
   for (;;)
   {
      label_node * lbl = *plbl;

      if (lbl)
      {
         int cmp = label_name[0] - lbl->name[0];

         if (cmp == 0)
            cmp = strcmp(label_name + 1, lbl->name + 1);
         if (cmp < 0)
            plbl = &lbl->left;
         else if (cmp > 0)
            plbl = &lbl->right;
         else
         {
            /* Write the label to file */
            
#line 6484 "nuweb.w"
            write_single_scrap_ref(file, lbl->scrap);
            fprintf(file, "-%02d", lbl->seq);
            break;
         }
      }
      else
      {
          /* Complain about missing label */
          
#line 6488 "nuweb.w"
          fprintf(stderr, "Can't find label %s.\n", label_name);
          break;
      }
   }
}

#line 6476 "nuweb.w"

