
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdarg.h>

//Some external
extern char *itoa(int, char *, int);

//A set of constant and global variable
#define MAX_BUFFERSIZE 1000000
#define MAX_VARG 9
#define LEN 100 // used for temp string
#define FALSE 0
#define TRUE 1
unsigned char outstr[MAX_BUFFERSIZE];

#define STR_DEFAULTPOLICE "Arial, Verdana, sans-serif"
#define STR_DEFAULTCOLOR "#ffffff"
#define STR_DEFAULTBACKGROUNDCOLOR "rgb(100,100,100)"
#define VAL_DEFAULTPOLICESIZE 12

#define STR_RESOURCE "resource"
#define STR_TYPE "type"
#define STR_ID "id"
#define STR_TEXT "text"
#define STR_ROOTNAME "ROOT"
#define STR_EPILOGUE "epilogue"
#define STR_PROLOGUE "prologue"
#define STR_BOX      "box"
#define STR_TEMPLATE "boxtemplate"
#define STR_IMAGE    "image"
#define STR_POVRAY   "povray"
#define STR_PROCESSING   "processing"
#define POVRAY_CMD   "povray"
#define POVRAY_PRF   "/tmp/scene"


// global flag for debug
int notInLua = 0;

//XML Trees for content and resources
xmlNode *root_element_resHTML = NULL;
xmlNode *root_element_Content = NULL;
int fileNameIncrement =0;
//output file specification
#define NOM_FICHIER_OUT "outfile.html"
FILE *outfile =NULL;
//input file specification
#define NOM_FICHIER_IN "infile.xml"
FILE *infile =NULL;
xmlDocPtr doc = NULL; //building the XML file tree for the infile
//input file with html resources ($i are parameters)
#define NOM_FICHIER_RES "reshtml.xml"
FILE *resfile =NULL;
xmlDocPtr resdoc = NULL; 

//
// My small tree library
//
#define MAXCHILDREN 100
int idTreeNode = 0;
typedef struct tn{
  int id;
  char *name;
  xmlChar *content;
  int hidden; // indicate if box and children should be hidden.
  xmlChar *URI; //for later when we want a box to be a target
  // various node features
  // no need for different types of node
  // not enough of them so it worth saving memory
  int xcoord;
  int ycoord;
  int zcoord;
  int xsize;
  int ysize;
  xmlChar *color;
  // box style
  xmlChar *stylename;
  xmlChar *style;
  xmlChar *script;
  xmlChar *imagefile;
  xmlChar *povray;
  xmlChar *processing;
  //pointers in the tree
  struct tn *parent;
  int nbchildren;
  struct tn *children[MAXCHILDREN];
  struct contextInformation *localContext;
} treenode;

treenode *newNode(treenode *parent, char *name){
  treenode *n = (treenode *) malloc(sizeof(treenode));
  if (parent) {
    if (parent->nbchildren >= MAXCHILDREN)  return NULL;
    parent->children[parent->nbchildren] = n;
    parent->nbchildren++;
  }
  n->id = idTreeNode; idTreeNode++;
  n->name = name;
  n->content = NULL;
  n->URI = NULL;
  n->hidden =FALSE;
  n->parent = parent;
  n->nbchildren = 0;
  n->xcoord = 0;
  n->ycoord = 0;
  n->zcoord = 0;
  n->xsize = 0;
  n->ysize = 0;
  n->stylename = NULL;
  n->style = NULL;
  n->script = NULL;
  n->imagefile = NULL;
  n->color= NULL;
  n->povray =NULL;
  n->processing =NULL;
  n->localContext =NULL;
  for (int i = 0; i < MAXCHILDREN; i++)  n->children[i] = NULL;
  return n;
}

//TODO to unlink node if already linked....
treenode *linkNode(treenode *parent, treenode *child){
  if (!parent || !child) return NULL;
  if (parent->nbchildren >= MAXCHILDREN)  return NULL;
  parent->children[parent->nbchildren] = child;
  parent->nbchildren++;
  child->parent = parent;
  return child;
}

void treeDisplay(treenode *n){
  if (!n) return;
  if (!n->parent) printf("node %i named %s\n", n->id,n->name);
  else printf("node %i named %s (parent %i) \n", n->id,n->name,n->parent->id);
  if (n->content) printf("\t CONTENT: %s\n",n->content);
  if (n->stylename) printf("\t STYLENAME: %s\n",n->stylename);
  if (n->imagefile) printf("\t IMAGEFILE: %s\n",n->imagefile);
  for (int i = 0; i < n->nbchildren; i++)  treeDisplay(n->children[i]);
}

void nodeSetContent(treenode *n, xmlChar *content){
  if (!n) return;
  n->content = content;
}

xmlChar * nodeGetContent(treenode *n){
 if (!n) return NULL;
 return  n->content;
}

//
// the context information for the page definition and layout
// right now this is in prologue information.
// later on this will be modifiable from the lua code
//
struct contextInformation {
  int sizeX;
  int sizeY;
  char *defaultPolice;
  char *defaultPoliceColor;
  char *backgroundColor;
  double  defaultPoliceSize;
  treenode *root;
} pageContext;


treenode * getNodeWithId(treenode *n, int id){
  if (!n) return NULL;
  if (n->id == id) return n;
  for (int i = 0; i < n->nbchildren; i++)  {
    treenode *res =  getNodeWithId(n->children[i],id);
    if (res) return res;
  }
  return NULL;
}



int genhtml_parent(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = getNodeWithId(pageContext.root,id);
  if (n && n->parent) {
    lua_pushnumber(L,n->parent->id);
  } else { // return itself in this case so it can be detected there is an issue.
    if (n) lua_pushnumber(L,n->id);
    else return 0;
  }
  return 1;
}

int genhtml_children(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the node id
  int c = luaL_checkinteger(L,2); // get the child number
  treenode *n = getNodeWithId(pageContext.root,id);
  if (n && (c >= 0) && (c <MAXCHILDREN) && n->children[c]) {
    lua_pushnumber(L,n->children[c]->id);
  } else { // return itself in this case so it can be detected there is an issue.
    if (n) lua_pushnumber(L,n->id);
    else return 0;
  }
  return 1;
}



treenode * getNodeWithName(treenode *n, char *name){
  if (!n || !name) return NULL;
  if (!strcmp(n->name,name)) return n;
  for (int i = 0; i < n->nbchildren; i++)  {
    treenode *res =  getNodeWithName(n->children[i],name);
    if (res) return res;
  }
  return NULL;
}

//THIS FUNCTION IS VERY MUCH MORE COMPLICATED TO USE A BROWSER TO GET IT...
int computePxHeigthOfaTextBox(int id, int textHeight){
  int height = textHeight;
  treenode * n = getNodeWithId(pageContext.root,id);
  if (!n) return 0;
  if (!n->content) return 0;
  if (!n->xsize) return 0;
  int endLine = n->xsize / textHeight; // NOT RIGHT BUT WILL DO FOR NOW
  int len = strlen((const char *) n->content);
  int countLine= 0;
  for(int i = 0; i <len; i++){
    char c = n->content[i];
    // let put something stupid for now.
    if (isascii(c)) {
      countLine++;
    }
    if (countLine > endLine){
      height+= textHeight;
      countLine = 0;
    }
  }
  return height;
}


void treeBoxEmit(treenode *n){
  if (!n) return;
  if (n->hidden) return;
  if (!strcmp(n->name,STR_BOX)){
      fprintf(outfile,"<!-- node %i named %s -->\n", n->id,n->name);
      if (n->style) fprintf(outfile,"%s\n",n->style);
      fprintf(outfile,"<div ");
      if (n->stylename) {
	fprintf(outfile,"class=\"%s\" ",n->stylename);
      } 
      if (n->xsize && n->ysize){
	fprintf(outfile,"style=\"position:absolute;width:%dpx;height:%dpx;left:%dpx;top:%dpx;z-index:%d\"",
		n->xsize,n->ysize,n->xcoord,n->ycoord,n->zcoord);
      }
      fprintf(outfile,">\n");
      if (n->content) fprintf(outfile,"%s\n",n->content);
      fprintf(outfile,"</div>\n");
  } else if (!strcmp(n->name,STR_IMAGE) || !strcmp(n->name,STR_POVRAY)){
      fprintf(outfile,"<!-- node %i named %s -->\n", n->id,n->name);
      if (n->style) fprintf(outfile,"%s\n",n->style);
      if (n->stylename) fprintf(outfile,"<div class=\"%s\">\n",n->stylename);
      if (n->imagefile) fprintf(outfile,"<img src=\"%s\" style=\"position:absolute;width:%dpx;height:%dpx;left:%dpx;top:%dpx;z-index:%d\"/>\n",
				n->imagefile,n->xsize,n->ysize,n->xcoord,n->ycoord,n->zcoord);
      if (n->stylename) fprintf(outfile,"</div>\n");
  } 
  for (int i = 0; i < n->nbchildren; i++)  treeBoxEmit(n->children[i]);
}

//support function to replace $i parameters in a string. Generate a new string. 
// size of the string is limited to ...
// consider parameter $1 -- $9... may work with more depends on atoi...
unsigned char *insertParam(const unsigned char *str, int nbarg, ...){
  const unsigned char *p = NULL;
  unsigned char *s = NULL;
  int cpt=0,i,j;
  va_list argp;
  int nbparam = 0;
  unsigned char *args[MAX_VARG+1];

  va_start(argp, nbarg);
  for (i=0; (i<nbarg) && (i<MAX_VARG);i++){
    s = va_arg(argp, unsigned char *);
    args[i] = s; 
  }
  bzero(outstr,MAX_BUFFERSIZE);
  for(p = str; *p != '\0'; p++){
    if(*p != '$'){
      outstr[cpt] = *p;
      cpt++;
      if (cpt >= MAX_BUFFERSIZE) return outstr;
      continue;
    } else {
      s = NULL;
      p++;
      j = atoi((const char *)p);
      if ((j> nbarg) || (j<1)) return outstr;
      s = args[j-1];
      if (s){
	for (; (cpt < MAX_BUFFERSIZE) && (*s != '\0') ; cpt++, s++) {
	  outstr[cpt] = *s;
	}
      }
    }
  }
  va_end(argp);
  return outstr;
}



//example fonction for XML tree
static void
print_element_names(xmlNode * a_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      printf("node type: Element, name: %s\n", cur_node->name);
      if (!strcmp((const char *) cur_node->name, STR_RESOURCE)){
	xmlChar* content = xmlNodeGetContent(cur_node);
	if (content){
	  printf("\tcontent: %s\n",insertParam(content,3,"param 1","param 2 é è §","last param"));     
	}
      }
    }
    xmlAttr* attribute = cur_node->properties;
    while(attribute && attribute->name && attribute->children){
	xmlChar* value = xmlNodeListGetString(cur_node->doc, attribute->children, 1);
	printf("\tattribute: %s \t%s\n",attribute->name,value);
	//do something with value
	xmlFree(value); 
	attribute = attribute->next;
    }
    print_element_names(cur_node->children);
  }
}

//look for an element value in an XML tree

xmlChar *
get_element_value(xmlNode * a_node, const char *id){
  xmlNode *cur_node = NULL;
  xmlChar *res = NULL;
  if (!a_node || !id) return NULL;
  //printf("looking for %s\n",id);
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      if (!strcmp((const char *) cur_node->name, STR_RESOURCE)){
	xmlChar* content = xmlNodeGetContent(cur_node);
	//if (content){
	//  printf("\tcontent: %s\n",insertParam(content,3,"param 1","param 2 é è §","last param"));     
	//}
	xmlAttr* attribute = cur_node->properties;
	while(attribute && attribute->name && attribute->children){
	  xmlChar* value = xmlNodeListGetString(cur_node->doc, attribute->children, 1);
	  //printf("\tattribute: %s \t%s\n",attribute->name,value);
	  if (!strcmp((const char *) attribute->name, STR_ID) && value && !strcmp((const char *)value,id)) {
	    xmlFree(value);
	    return content;
	  }
	  attribute = attribute->next;
	}
      }
    }
    res = get_element_value(cur_node->children,id);
    if (res) return res;
  }
  return NULL;
}


//show a ressource
int genhtml_show(lua_State *L){
  const char *s = luaL_checkstring(L,1); // get the first argument
  if (s) {
    printf("looking for %s\n",s);
    xmlChar *r = get_element_value(root_element_Content,s);
    if (r) printf("%s\n",r);
    else {
      r = get_element_value(root_element_resHTML,s);
      if (r) printf("%s\n",r);
      else printf("Resource not found\n");
    }
    return 0;
  } else {
    printf("Resource not found\n");
    return 0; // how many things on the stack when returning
  }
}

//show a ressource
int genhtml_resource(lua_State *L){
  const char *s = luaL_checkstring(L,1); // get the first argument
  if (s) {
    //printf("looking for %s\n",s);
    xmlChar *r = get_element_value(root_element_Content,s);
    if (r) {
      //printf("%s\n",r);
      lua_pushstring(L, (const char *) r);
       return 1;
    } else {
      r = get_element_value(root_element_resHTML,s);
      if (r) {
	//printf("%s\n",r);
	lua_pushstring(L, (const char *) r);
	return 1;
      } else 
	printf("Resource not found\n");
    }
    return 0;
  } else {
    printf("Resource not found\n");
    return 0; // how many things on the stack when returning
  }
}


int genhtml_hide(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the box id
  treenode *n = getNodeWithId(pageContext.root,id);
  if (n) n->hidden = TRUE;
  return 0;
}


int genhtml_visible(lua_State *L){
  int id = luaL_checkinteger(L,1); 
  treenode *n = getNodeWithId(pageContext.root,id);
  if (n) n->hidden = FALSE;
  return 0;
}


//create a generic box to display on the page
// must be attached to a node, root is the default
int genhtml_createbox(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = newNode(getNodeWithId(pageContext.root,id),STR_BOX);
  lua_pushnumber(L,n->id);
  return 1;
}

//create a generic image box to display on the page
int genhtml_createimage(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  const char *s = luaL_checkstring(L,2); // get the file of the image
  treenode *n = newNode(getNodeWithId(pageContext.root,id),STR_IMAGE);
  n->imagefile = (xmlChar *) s;
  lua_pushnumber(L,n->id);
  return 1;
}

int genhtml_createscene(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = newNode(getNodeWithId(pageContext.root,id),STR_POVRAY);
  lua_pushnumber(L,n->id);
  return 1;
}


//place the box in the space X,Y,Z
int genhtml_place(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the id of the node
  int X = luaL_checkinteger(L,2); // X
  int Y= luaL_checkinteger(L,3); // Y
  int Z= luaL_checkinteger(L,4); // Z
  treenode *n = getNodeWithId(pageContext.root,id);
  if (n && n->parent){
    n->xcoord = X+n->parent->xcoord;
    n->ycoord = Y+n->parent->ycoord;
    n->zcoord = Z+n->parent->zcoord;
  } else if (n){
    n->xcoord = X;
    n->ycoord = Y;
    n->zcoord = Z;
  }
  return 0;
}

//size of the box
int genhtml_size(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the id of the node
  int X = luaL_checkinteger(L,2); // X
  int Y= luaL_checkinteger(L,3); // Y
  treenode *n = getNodeWithId(pageContext.root,id);
  if (n){
    n->xsize= X;
    n->ysize = Y;
  }
  return 0;
}

//show a ressource
int genhtml_shownode(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  if (id) {
    printf("node %d\n",id);
    treenode *n = getNodeWithId(pageContext.root,id);
    if (!n){
      printf("Node not found\n");
      return 0;
    }
    xmlChar *r =  nodeGetContent(n);
    if (r) printf("%s\n",r);
    else  printf("node has no content\n");
    if (n->content) printf("\t CONTENT: %s\n",n->content);
    if (n->stylename) printf("\t STYLENAME: %s\n",n->stylename);
    return 0; // how many things on the stack when returning
  }
  return 0;
}

//list of constant used for generating the HTML file
int genhtml_add1(lua_State *L){
  double d = luaL_checknumber(L,1); // get the first argument
  lua_pushnumber(L,d+1); // return the result
  return 1; // how many things on the stack when returning
}

//set page witdh
//list of constant used for generating the HTML file
int genhtml_setpagewidth(lua_State *L){
  int w = luaL_checkinteger(L,1); // get the first argument
  pageContext.sizeX = w;
  return 0; 
}

//set page height
//list of constant used for generating the HTML file
int genhtml_setpageheight(lua_State *L){
  int h = luaL_checkinteger(L,1); // get the first argument
  pageContext.sizeY = h;
  return 0; 
}

int genhtml_start(lua_State *L){
  printf("Opening output file\n");
  if (outfile == NULL){
    outfile = fopen(NOM_FICHIER_OUT, "wb");
    // start to add trees nodes
    xmlChar *epilogue = get_element_value(root_element_resHTML,STR_EPILOGUE);
    xmlChar *prologue = get_element_value(root_element_resHTML,STR_PROLOGUE);
    if (!epilogue) printf("No epilogue given\n");
    if (!prologue) printf("No prologue given\n");
    treenode *nepilogue = newNode(pageContext.root,STR_EPILOGUE);
    nodeSetContent(nepilogue,epilogue);
    treenode *nprologue = newNode(pageContext.root,STR_PROLOGUE);
    nodeSetContent(nprologue,prologue);
    //treeDisplay(pageContext.root);
  } else 
    printf("already started\n");
  return 0; 
}


int genhtml_finish(lua_State *L){
  if (outfile != NULL){
    // We do not print HTM before the end... 
    printf("Generating HTML file in %s\n",NOM_FICHIER_OUT);
    //start to fill the HTML FILE
    // generate prologue
    // prologue has one parameter, the width of the screen in px
    treenode *nprologue = getNodeWithName(pageContext.root,STR_PROLOGUE);
    if (!nprologue) printf("Prologue node not found\n");
    xmlChar *content = nodeGetContent(nprologue);
    if (content){
      char w[LEN];
			char h[LEN];
      // first parameter is the width in px
      snprintf(w, LEN, "%d", pageContext.sizeX);
			snprintf(h, LEN, "%d", pageContext.sizeY);
      fprintf(outfile,"%s\n",insertParam(content,5,w,h,STR_DEFAULTPOLICE,STR_DEFAULTCOLOR,STR_DEFAULTBACKGROUNDCOLOR));
    }
    //body generation
    treeBoxEmit(pageContext.root);
    //generate epilogue
    treenode *nepilogue = getNodeWithName(pageContext.root,STR_EPILOGUE);
    if (!nepilogue) printf("Epilogue node not found\n");
    content = nodeGetContent(nepilogue);
    if (content){
      fprintf(outfile,"%s\n",content);
    }
    printf("Closing output file\n");
    fclose(outfile);
    outfile = NULL;
  }
  return 0; 
}


int genhtml_setboxcontent(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = getNodeWithId(pageContext.root,id);
  if (!n) return 0;
  const char *s = luaL_checkstring(L,2); // content 
  if (s) {
    nodeSetContent(n,(xmlChar *) s);
  }
  return 0;
}

int genhtml_setboxstylename(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = getNodeWithId(pageContext.root,id);
  if (!n) return 0;
  const char *s = luaL_checkstring(L,2); // content 
  if (s) {
    n->stylename = (xmlChar *) s;
  }
  return 0;
}

int genhtml_setboxstyle(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = getNodeWithId(pageContext.root,id);
  if (!n) return 0;
  const char *s = luaL_checkstring(L,2); // content 
  if (s) {
    n->style = (xmlChar *) s;
  }
  return 0;
}


int genhtml_setpovray(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = getNodeWithId(pageContext.root,id);
  if (!n) return 0;
  const char *s = luaL_checkstring(L,2); // content 
  if (s) {
    n->povray = (xmlChar *) s;
  }
  return 0;
}

int genhtml_render(lua_State *L){
  int id = luaL_checkinteger(L,1); // get the first argument
  treenode *n = getNodeWithId(pageContext.root,id);
  if (!n) return 0;
  if (!n->povray) return 0;
  char *filenameout = (char *) malloc(sizeof(POVRAY_PRF)+32);
  char *filenamein = (char *) malloc(sizeof(POVRAY_PRF)+32);
  sprintf(filenameout,"%s_%d.png",POVRAY_PRF,fileNameIncrement);
  sprintf(filenamein,"%s_%d.pov",POVRAY_PRF,fileNameIncrement);
  fileNameIncrement++;
  n->imagefile= (xmlChar *) filenameout;
  FILE * scenefile = fopen(filenamein, "wb");
  fprintf(scenefile,"%s\n",n->povray);
  fclose(scenefile);
  char *cmd = malloc(MAX_BUFFERSIZE);
  sprintf(cmd,"%s  %s &> /dev/null",POVRAY_CMD,filenamein);
  printf("rendering %s\n",cmd);
  system(cmd);
  free(cmd);
  return 0;
}


int genhtml_tree(lua_State *L){
  treeDisplay(pageContext.root);
  return 0;
}

// argument is str to instanciate, next number of param, then string value of the param
int genhtml_instanciate(lua_State *L){
  const char *p[MAX_VARG];
  const char *s = luaL_checkstring(L,1); 
  int nium = luaL_checkinteger(L,2);
  for (int i=0; (i<nium) && (i<MAX_VARG); i++){
    p[i] = luaL_checkstring(L,3+i);
  }
  
  switch(nium){
  case 1: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,1,p[0]))); return 1;
  case 2: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,2,p[0],p[1]))); return 1;
  case 3: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,3,p[0],p[1],p[2]))); return 1;
  case 4: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,4,p[0],p[1],p[2],p[3]))); return 1;
  case 5: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,5,p[0],p[1],p[2],p[3],p[4]))); return 1;
  case 6: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,6,p[0],p[1],p[2],p[3],p[4],p[5]))); return 1;
  case 7: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,7,p[0],p[1],p[2],p[3],p[4],p[5],p[6]))); return 1;
  case 8: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,8,p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]))); return 1;
  case 9: lua_pushstring(L,strdup((const char *) insertParam((const unsigned char *) s,9,p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8]))); return 1;
  }
  lua_pushstring(L,s);
  return 1;
}

int genhtml_root(lua_State *L){
  lua_pushnumber(L,0); // return the result
  return 1; // how many things on the stack when returning
}

int genhtml_boxheight(lua_State *L){
  int id = luaL_checkinteger(L,1); 
  int h = computePxHeigthOfaTextBox(id,pageContext.defaultPoliceSize);
  lua_pushnumber(L, h);
  return 1;
 }

/////////// INITIALISATION TASK
static const struct luaL_reg genhtmllib[] ={
  {"add1", genhtml_add1},
  {"start", genhtml_start},
  {"finish", genhtml_finish},
  {"show",genhtml_show},
  {"shownode",genhtml_shownode},
  {"box",genhtml_createbox},
  {"image",genhtml_createimage},
  {"scene",genhtml_createscene},
  {"setboxcontent",genhtml_setboxcontent},
  {"setboxstylename",genhtml_setboxstylename},
  {"resource", genhtml_resource},
  {"instanciate", genhtml_instanciate},
  {"setboxstyle",genhtml_setboxstyle},
  {"setpovray",genhtml_setpovray},
  {"root",genhtml_root},
  {"place",genhtml_place},
  {"size",genhtml_size},
  {"tree",genhtml_tree},
  {"render",genhtml_render},
  {"hide",genhtml_hide},
  {"visible",genhtml_visible},
  {"parent",genhtml_parent},
  {"children",genhtml_children},
  {"setpagewidth",genhtml_setpagewidth},
	{"setpageheight",genhtml_setpageheight},
  {"boxheight",genhtml_boxheight},
  {NULL,NULL}
};


// register the library
int luaopen_genhtml(lua_State *L){
  xmlParserCtxtPtr ctxt; /* the parser context */

  if (!notInLua){
    luaL_register(L,"genhtml",genhtmllib);
  }
  //read resources
  ctxt = xmlNewParserCtxt();
  if (ctxt == NULL) {
    fprintf(stderr, "Failed to allocate parser context\n");
    return 1;
  }
  doc = xmlCtxtReadFile(ctxt,NOM_FICHIER_IN, NULL,0);
  if (doc == NULL) {
    fprintf(stderr, "Failed to parse input document\n");
  } else {
    root_element_Content = xmlDocGetRootElement(doc);
    // print_element_names(root_element_Content);
  }
  resdoc = xmlCtxtReadFile(ctxt,NOM_FICHIER_RES, NULL,0);
  if (resdoc == NULL) {
    fprintf(stderr, "Failed to parse HTML resources document\n");
  } else {
    root_element_resHTML = xmlDocGetRootElement(resdoc);
    //    print_element_names(root_element_resHTML);
  }
  xmlFreeParserCtxt(ctxt);

  // set the default layout
  pageContext.sizeX = 480;
  pageContext.sizeY = 320;
  pageContext.defaultPolice = NULL;
  pageContext.backgroundColor = NULL;
  pageContext.defaultPoliceColor = NULL;
  pageContext.defaultPoliceSize = VAL_DEFAULTPOLICESIZE;
  pageContext.root = newNode(NULL,STR_ROOTNAME);
  return 1;
}

