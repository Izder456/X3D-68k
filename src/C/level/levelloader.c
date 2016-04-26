// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <ctype.h>
#include <string.h>

#include "X3D_common.h"
#include "X3D_segmentbuilder.h"

enum {
  TYPE_INT,
  TYPE_STR,
  TYPE_OP,
  TYPE_ID
};

typedef struct X3D_LevelToken {
  int16 type;
  char str[128];
} X3D_LevelToken;

enum {
  ARG_INT,
  ARG_VEX3D
};

uint16 seg_id[256];

typedef struct X3D_CmdArg {
  char name[128];
  int16 type;
  _Bool set;
  
  union {
    int32 i_val;
    X3D_Vex3D v3d_val;
  };
} X3D_CmdArg;

typedef struct X3D_LevelVar {
  char name[128];
} X3D_LevelVar;

void x3d_level_consume_whitespace(char** start, char* end) {
  while(*start < end && isspace(**start))
    ++(*start);
}

_Bool x3d_level_lex_int(char** start, char* end, X3D_LevelToken* token) {
  char* s = *start;
  char* t_str = token->str;
  
  if(!isdigit(*s) && *s != '-')
    return X3D_FALSE;
  
  token->type = TYPE_INT;
  
  do {
    *t_str = *s;
    ++t_str;
  } while(++s < end && isdigit(*s));
  
  *t_str = '\0';
  *start = s;
  
  return X3D_TRUE;
}

_Bool x3d_level_lex_ident(char** start, char* end, X3D_LevelToken* token) {
  char* s = *start;
  char* t_str = token->str;
  
  if(!isalpha(*s))
    return X3D_FALSE;
  
  token->type = TYPE_ID;
  
  do {
    *t_str = *s;
    ++t_str;
  } while(++s < end && (*s == '_' || isalnum(*s)));
  
  *t_str = '\0';
  *start = s;
  
  return X3D_TRUE;
}


_Bool x3d_level_lex_op(char** start, char* end, X3D_LevelToken* token) {
  if(**start == '=' || **start == '{' || **start == '}' || **start == ',') {
    token->str[0] = **start;
    token->str[1] = '\0';
    token->type = TYPE_OP;
    ++(*start);
    
    return X3D_TRUE;
  }
  
  return X3D_FALSE;
}

_Bool x3d_level_lex_token(char** start, char* end, X3D_LevelToken* token) {
  x3d_level_consume_whitespace(start, end);
  
  if(*start == end)
    return X3D_FALSE;
  
  _Bool pass = x3d_level_lex_int(start, end, token) ||
    x3d_level_lex_ident(start, end, token)    ||
    x3d_level_lex_op(start, end, token);
    
  if(!pass) {
    x3d_log(X3D_ERROR, "        Bad token starting at '%s'", *start);
  }
  
  return pass;
}

static _Bool expect_op(X3D_LevelToken* tokens, uint16 t, uint16 total_t, char* op) {
  if(t >= total_t || strcmp(tokens[t].str, op) != 0) {
    x3d_log(X3D_ERROR, "        Expected '%s' (got '%s')", op, tokens[t].str);
    return X3D_FALSE;
  }
  
  return X3D_TRUE;
}

static _Bool expect_int(X3D_LevelToken* tokens, uint16 t, uint16 total_t, int32* res) {
  if(t >= total_t) {
    x3d_log(X3D_ERROR, "        Not enough tokens (expected int)");
    return X3D_FALSE;
  }
  
  if(tokens[t].type != TYPE_INT) {
    x3d_log(X3D_ERROR, "        Expected int (got '%s')", tokens[t].str);
    return X3D_FALSE;
  }
  
  *res = atoi(tokens[t].str);
  
  return X3D_TRUE;
}

static _Bool expect_vex3d(X3D_LevelToken* tokens, uint16 t, uint16 total_t, X3D_Vex3D* res) {
  if(total_t < 7) {
    x3d_log(X3D_ERROR, "        Expected vex3d");
    return X3D_FALSE;
  }
  
  tokens += t;
  
  X3D_Vex3D_int32 v;
  
  _Bool pass = expect_op(tokens, 0, total_t, "{") &&
    expect_int(tokens, 1, total_t, &v.x) &&
    expect_op(tokens, 2, total_t, ",") &&
    expect_int(tokens, 3, total_t, &v.y) &&
    expect_op(tokens, 4, total_t, ",") &&
    expect_int(tokens, 5, total_t, &v.z) &&
    expect_op(tokens, 6, total_t, "}");
    
  if(pass) {
    res->x = v.x;
    res->y = v.y;
    res->z = v.z;
  }
  
  return pass;
}

_Bool parse_arg(X3D_LevelToken** token, X3D_LevelToken* end, X3D_CmdArg* args, uint16 total_a) {
  X3D_LevelToken* t = *token;
  
  if(end - *token < 3) {
    x3d_log(X3D_INFO, "        Expected argument assignment");
    return X3D_FALSE;
  }
  
  if(t[0].type != TYPE_ID) {
    x3d_log(X3D_ERROR, "        Expected command arg");
    return X3D_FALSE;
  }
  if(t[1].type != TYPE_OP || strcmp(t[1].str, "=") != 0) {
    x3d_log(X3D_ERROR, "        Expected '='");
    return X3D_FALSE;
  }
  
  uint16 i;
  for(i = 0; i < total_a; ++i) {
    if(strcmp(t[0].str, args[i].name) == 0) {
      if(args[i].type == ARG_INT) {
        if(!expect_int(t, 2, end - *token, &args[i].i_val)) {
          return X3D_FALSE;
        }
        
        args[i].set = X3D_TRUE;
        *token += 3;
      }
      else if(args[i].type == ARG_VEX3D) {
        if(!expect_vex3d(t, 2, end - *token, &args[i].v3d_val)) {
          return X3D_FALSE;
        }
        
        args[i].set = X3D_TRUE;
        *token += 9;
      } 
      
      return X3D_TRUE;
    }
  }
  
  x3d_log(X3D_INFO, "        Unknown arg: %s", t[0].str);
  
  return X3D_FALSE;
}

_Bool parse_args(X3D_LevelToken** token, X3D_LevelToken* end, X3D_CmdArg* args, uint16 total_a) {
  while(*token < end) {
    if(!parse_arg(token, end, args, total_a))
      return X3D_FALSE;
  }
  
  return X3D_TRUE;
}

uint16 add_int_arg(X3D_CmdArg* args, char* name, uint16* total_a, int32 def) {
  strcpy(args[*total_a].name, name);
  args[*total_a].type = ARG_INT;
  args[*total_a].i_val = def;
  args[*total_a].set = X3D_FALSE;
  
  return (*total_a)++;
}

uint16 add_vex3d_arg(X3D_CmdArg* args, char* name, uint16* total_a, X3D_Vex3D def) {
  strcpy(args[*total_a].name, name);
  args[*total_a].type = ARG_VEX3D;
  args[*total_a].v3d_val = def;
  args[*total_a].set = X3D_FALSE;
  
  return (*total_a)++;
}

_Bool set_seg_id(uint16 id, uint16 val) {
  if(id >= 256) {
    x3d_log(X3D_INFO, "        Seg id too big (max 255): %d", id);
    return X3D_FALSE;
  }
  
  seg_id[id] = val;
  return X3D_TRUE;
}

_Bool get_seg_id(uint16 id, uint16* dest) {
  if(id >= 256) {
    x3d_log(X3D_INFO, "        Seg id too big (max 255): %d", id);
    return X3D_FALSE;
  }
  
  if(seg_id[id] == X3D_SEGMENT_NONE) {
    x3d_log(X3D_INFO, "        No such seg: %d", id);
    return X3D_FALSE;
  }
  
  *dest = seg_id[id];
  return X3D_TRUE;
}

//=============================================================================

_Bool x3d_level_cmd_addseg(X3D_LevelToken* tokens, uint16 total_t) {
  X3D_CmdArg args[10];
  uint16 total_a = 0;
  
  add_int_arg(args, "v", &total_a, 4);
  add_int_arg(args, "r", &total_a, 200);
  add_int_arg(args, "h", &total_a, 200);
  add_vex3d_arg(args, "pos", &total_a, (X3D_Vex3D) { 0, 0, 0 });
  add_vex3d_arg(args, "angle", &total_a, (X3D_Vex3D) { 0, 0, 0 });
  add_int_arg(args, "id", &total_a, -1);
  
  X3D_Vex3D_angle256 angle = { args[4].v3d_val.x % 256, args[4].v3d_val.y % 256, args[4].v3d_val.z % 256 };
  
  if(!parse_args(&tokens, tokens + total_t, args, total_a))
    return X3D_FALSE;
  
  X3D_Prism3D* prism = alloca(1000);
  x3d_prism3d_construct(prism, args[0].i_val, args[1].i_val, args[2].i_val, angle);
  
  uint16 i;
  for(i = 0; i < args[0].i_val * 2; ++i) {
    prism->v[i] = x3d_vex3d_add(prism->v + i, &args[3].v3d_val);
  }
  
  uint16 id = x3d_segmentbuilder_add_uncompressed_segment(prism)->base.id;
  
  if(args[5].set) {
    return set_seg_id(args[5].i_val, id);
  }
  
  return X3D_TRUE;
}

_Bool x3d_level_cmd_connect(X3D_LevelToken* tokens, uint16 total_t) {
  X3D_CmdArg args[10];
  uint16 total_a = 0;
  
  add_int_arg(args, "s1", &total_a, -1);
  add_int_arg(args, "f1", &total_a, -1);
  add_int_arg(args, "s2", &total_a, -1);
  add_int_arg(args, "f2", &total_a, -1);
  
  if(!parse_args(&tokens, tokens + total_t, args, total_a))
    return X3D_FALSE;
  
  uint16 i;
  for(i = 0; i < 4; ++i) {
    if(!args[i].set) {
      x3d_log(X3D_INFO, "        Missing arg: '%s'", args[i].name);
      return X3D_FALSE;
    }
  }
  
  uint16 s1, s2;
  
  if(!get_seg_id(args[0].i_val, &s1) || !get_seg_id(args[2].i_val, &s2))
    return X3D_FALSE;

  x3d_segmentbuilder_add_connecting_segment(
    x3d_segfaceid_create(s1, args[1].i_val),
    x3d_segfaceid_create(s2, args[3].i_val)
  );
  
  return X3D_TRUE;
}

_Bool x3d_level_cmd_connect_close(X3D_LevelToken* tokens, uint16 total_t) {
  X3D_CmdArg args[10];
  uint16 total_a = 0;
  
  add_int_arg(args, "s1", &total_a, -1);
  add_int_arg(args, "s2", &total_a, -1);
  
  if(!parse_args(&tokens, tokens + total_t, args, total_a))
    return X3D_FALSE;
  
  uint16 i;
  for(i = 0; i < 2; ++i) {
    if(!args[i].set) {
      x3d_log(X3D_INFO, "        Missing arg: '%s'", args[i].name);
      return X3D_FALSE;
    }
  }
  
  uint16 s1, s2;
  
  if(!get_seg_id(args[0].i_val, &s1) || !get_seg_id(args[1].i_val, &s2))
    return X3D_FALSE;

  X3D_Segment* seg1 = x3d_segmentmanager_load(s1);
  X3D_Segment* seg2 = x3d_segmentmanager_load(s2);
  
  X3D_Polygon3D poly1 = {
    .v = alloca(1000)
  };
  
  X3D_Polygon3D poly2 = {
    .v = alloca(1000)
  };
  
  int32 min_dist = 0x7FFFFFF;
  int16 min_i = -1;
  int16 min_d = -1;
  
  uint16 d;
  for(i = 0; i < seg1->prism.base_v + 2; ++i) {
    for(d = 0; d < seg2->prism.base_v + 2; ++d) {
      x3d_prism3d_get_face(&seg1->prism, i, &poly1);
      x3d_prism3d_get_face(&seg2->prism, d, &poly2);
      
      if(poly1.total_v != poly2.total_v)
        continue;
      
      X3D_Vex3D c1, c2;
      x3d_polygon3d_center(&poly1, &c1);
      x3d_polygon3d_center(&poly2, &c2);
      
      X3D_Vex3D diff;
      diff = x3d_vex3d_sub(&c1, &c2);
      
      int32 dist = x3d_vex3d_int16_mag(&diff);
      
      if(dist < min_dist) {
        min_dist = dist;
        min_i = i;
        min_d = d;
      }
    }
  }
  
  
  x3d_segmentbuilder_add_connecting_segment(
    x3d_segfaceid_create(s1, min_i),
    x3d_segfaceid_create(s2, min_d)
  );
  
  return X3D_TRUE;
}

//=============================================================================

#define MATCH(_str, _num) if(strcmp(s, _str) == 0) return _num;

enum {
  CMD_ADDSEG,
  CMD_CONNECT,
  CMD_CONNECT_CLOSE
};

_Bool ((*callback[])(X3D_LevelToken* tokens, uint16 total_t)) = {
  x3d_level_cmd_addseg,
  x3d_level_cmd_connect,
  x3d_level_cmd_connect_close
};

int16 x3d_level_command_code(char* s) {
  MATCH("addseg", CMD_ADDSEG);
  MATCH("connect", CMD_CONNECT);
  MATCH("connect_close", CMD_CONNECT_CLOSE)
  
  return -1;
}

_Bool x3d_level_execute_command(X3D_LevelToken* tokens, uint16 total_t) {
  if(total_t == 0)
    return X3D_TRUE;
  
  if(tokens[0].type != TYPE_ID) {
    x3d_log(X3D_ERROR, "        Expected command as first arg");
    return X3D_FALSE;
  }
  
  int16 code = x3d_level_command_code(tokens[0].str);
  
  if(code == -1)
    return X3D_FALSE;
  
  return callback[code](tokens + 1, total_t - 1);
}

_Bool x3d_level_run_command(char* str) {
  char* start = str;
  char* end = str + strlen(str);
  X3D_LevelToken tokens[100];
  uint16 total_t = 0;
  
  x3d_log(X3D_INFO, "%% %s", str);
  
  while(x3d_level_lex_token(&start, end, tokens + total_t)) {
    if(tokens[total_t].str[0] != '\0')
      ++total_t;
  }
  
  if(start != end) {
    x3d_log(X3D_ERROR, "        Bad command: %s", str);
    return X3D_FALSE;
  }
  
  if(!x3d_level_execute_command(tokens, total_t)) {
    x3d_log(X3D_ERROR, "        Error executing level command: %s", str);
    return X3D_FALSE;
  }
  
  return X3D_TRUE;
}

void x3d_level_command_init(void) {
  uint16 i;
  for(i = 0; i < 256; ++i) {
    seg_id[i] = X3D_SEGMENT_NONE;
  }
}

