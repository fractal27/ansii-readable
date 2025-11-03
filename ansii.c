#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"
/*  see `enum ansii_type` in header */
#include "ansii.h"

#define HASH_CHECK(a)    (key_hash == a)

struct ansii_t {
       char prefix_ch;
       union {
         unsigned int single;
         unsigned int pair[2];
         unsigned int triplet[3];
         unsigned int quintet[5];
       } value;
       char suffix_ch;
       enum {
         VALUE_SINGLE = 0,
         VALUE_PAIR,
         VALUE_TRIPLET,
         VALUE_QUINTET
       } value_type;
       bool valid;
       bool no_bracket;
};

struct hash {
       unsigned int hash;
       bool computed;
};


inline unsigned long
hash(char str[], struct hash* to_compute)
{
       if(to_compute && to_compute->computed){
              // log_verbose("[hash already computed] `%lx`\n",to_compute->hash);
              return to_compute->hash;
       }
       unsigned char* ustr = (unsigned char*)str;
       unsigned long hash = 5381;
       int c;

       while ((c = *ustr++))
         hash = ((hash << 5) + hash) + c; // hash * 33 + c 

       if(to_compute) {
              to_compute->computed = true;
              to_compute->hash = hash;
              // log_verbose("[hash computed] `%lx`\n",hash);
       }
       return hash;
}

struct ansii_t 
erase_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static bool hashes_initialized = false;

       static unsigned long  hash_erase_from_cursor_to_end, hash_erase_from_cursor_to_beginning,
              hash_erase_entire_screen, hash_erase_saved_lines, erase_from_cursor_to_endline,
              erase_from_start_line_to_cursor, erase_entire_line;
             

       if(!hashes_initialized){
         log_info("initializing hashes\n");
         hash_erase_from_cursor_to_end = hash("from-cursor-to-end",NULL);
         hash_erase_from_cursor_to_beginning = hash("from-cursor-to-beginning",NULL);
         hash_erase_entire_screen = hash("entire-screen",NULL);
         hash_erase_saved_lines = hash("saved-lines",NULL);
         erase_from_cursor_to_endline = hash("from-cursor-to-endline",NULL);
         erase_from_start_line_to_cursor = hash("from-start-line-to-cursor",NULL);
         erase_entire_line = hash("entire-line",NULL);
         hashes_initialized = true;
       }
       if HASH_CHECK(hash_erase_from_cursor_to_end)       return (struct ansii_t){.value.single=0,.suffix_ch='J',.valid=true};
       if HASH_CHECK(hash_erase_from_cursor_to_beginning) return (struct ansii_t){.value.single=1,.suffix_ch='J',.valid=true};
       if HASH_CHECK(hash_erase_entire_screen)       return (struct ansii_t){.value.single=2,.suffix_ch='J',.valid=true};
       if HASH_CHECK(hash_erase_saved_lines)         return (struct ansii_t){.value.single=3,.suffix_ch='J',.valid=true};
       if HASH_CHECK(erase_from_cursor_to_endline)        return (struct ansii_t){.value.single=0,.suffix_ch='K',.valid=true};
       if HASH_CHECK(erase_from_start_line_to_cursor)     return (struct ansii_t){.value.single=1,.suffix_ch='K',.valid=true};
       if HASH_CHECK(erase_entire_line)         return (struct ansii_t){.value.single=2,.suffix_ch='K',.valid=true};
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}


struct ansii_t
cursor_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static bool hashes_initialized = false;

       static unsigned long hash_visible, hash_invisible, hash_to_origin, hash_one_up, hash_save,
             hash_restore, hash_request_position;
       unsigned int* pair_to_read = malloc(sizeof(unsigned int)*2);
       unsigned int value_to_read;

       if(!hashes_initialized){
         log_info("initializing hashes\n");
         hash_visible = hash("visible",NULL);
         hash_invisible = hash("invisible",NULL);
         hash_to_origin = hash("to-origin",NULL);
         hash_one_up = hash("one-up",NULL);
         hash_request_position = hash("request-position",NULL);
         hash_save = hash("save",NULL);
         hash_restore = hash("restore",NULL);
         hashes_initialized = true;
       }
       // log_verbose("hash: %lu of (%lu; %lu)",key_hash,hash_visible, hash_invisible);
       if HASH_CHECK(hash_invisible)        return free(pair_to_read), (struct ansii_t){'?',.value.single=25,.suffix_ch='l',.valid=true}; // ?25h
       if HASH_CHECK(hash_visible)          return free(pair_to_read), (struct ansii_t){'?',.value.single=25,.suffix_ch='h',.valid=true};  // ?25l
       if HASH_CHECK(hash_restore)          return free(pair_to_read), (struct ansii_t){.suffix_ch='u',.valid=true};  // ?25l
       if HASH_CHECK(hash_to_origin)        return free(pair_to_read), (struct ansii_t){.suffix_ch='H',.valid=true};
       if HASH_CHECK(hash_one_up)           return free(pair_to_read), (struct ansii_t){.suffix_ch='M',.valid=true,.no_bracket=true};
       if HASH_CHECK(hash_save)             return free(pair_to_read), (struct ansii_t){.suffix_ch='s',.valid=true};
       if HASH_CHECK(hash_restore)          return free(pair_to_read), (struct ansii_t){.suffix_ch='u',.valid=true};
       if HASH_CHECK(hash_request_position) return free(pair_to_read), (struct ansii_t){.value.single=6,.suffix_ch='n',.valid=true};
       // finished static analisys
       struct ansii_t result = {};
       if(sscanf(key,"move-to-%u-%u",&pair_to_read[0],&pair_to_read[1])){
         result.suffix_ch = 'H';
         result.value_type = VALUE_PAIR;
         result.valid = true;
         if(memmove(result.value.pair,pair_to_read,2)){
           return result;
         }
         log_error("Memmove failed. pair not copied over");
       } else if(sscanf(key,"move-%u-up",&value_to_read)){    return (struct ansii_t){.value.single=value_to_read,.suffix_ch='A',.valid=true};
       } else if(sscanf(key,"move-%u-down",&value_to_read)){  return (struct ansii_t){.value.single=value_to_read,.suffix_ch='B',.valid=true};
       } else if(sscanf(key,"move-%u-right",&value_to_read)){ return (struct ansii_t){.value.single=value_to_read,.suffix_ch='C',.valid=true};
       } else if(sscanf(key,"move-%u-left",&value_to_read)){  return (struct ansii_t){.value.single=value_to_read,.suffix_ch='D',.valid=true};
       } else if(sscanf(key,"move-to-beginning-%u-lines-down",&value_to_read)){ return (struct ansii_t){.value.single=value_to_read,.suffix_ch='E',.valid=true};
       } else if(sscanf(key,"move-to-beginning-%u-lines-up",&value_to_read)){   return (struct ansii_t){.value.single=value_to_read,.suffix_ch='F',.valid=true};
       } else if(sscanf(key,"move-to-col-%u",&value_to_read)){        return (struct ansii_t){.value.single=value_to_read,.suffix_ch='G',.valid=true};
       }
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}

struct ansii_t
altbuf_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static bool hashes_initialized = false;

       static unsigned long hash_enable, hash_disable;
       if(!hashes_initialized){
         log_info("initializing hashes\n");
         hash_enable = hash("enable",NULL);
         hash_disable = hash("disable",NULL);
         hashes_initialized = true;
       }
       if HASH_CHECK(hash_enable)  return (struct ansii_t){'?',.value.single = 1049,.suffix_ch = 'h',.valid=true};
       if HASH_CHECK(hash_disable) return (struct ansii_t){'?',.value.single = 1049,.suffix_ch = 'l',.valid=true};
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}

struct ansii_t
scattr_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static bool hashes_initialized = false;

       static unsigned long hash_screen_40_x_25_monochrome_text, hash_screen_40_x_25_color_text, hash_screen_80_x_25_monochrome_text, hash_screen_80_x_25_color_text, hash_screen_320_x_200_4_color_graphics, hash_screen_320_x_200_monochrome_graphics, hash_screen_640_x_200_monochrome_graphics, hash_screen_linewrap, hash_screen_320_x_200_color_graphics, hash_screen_640_x_350_2_color_graphics, hash_screen_640_x_350_16_color_graphics, hash_screen_640_x_480_2_color_graphics, hash_screen_640_x_480_16_color_graphics, hash_screen_320_x_200_256_color_graphics, hash_screen_restore, hash_screen_save;



       if(!hashes_initialized){
         log_info("initializing hashes\n");
         hash_screen_40_x_25_monochrome_text        = hash("40x25 monochrome (text)",NULL);
         hash_screen_40_x_25_color_text             = hash("40x25 color (text)",NULL);
         hash_screen_80_x_25_monochrome_text        = hash("80x25 monochrome (text)",NULL);
         hash_screen_80_x_25_color_text             = hash("80x25 color (text)",NULL);
         hash_screen_320_x_200_4_color_graphics     = hash("320x200 4-color (graphics)",NULL);
         hash_screen_320_x_200_monochrome_graphics  = hash("320x200 monochrome (graphics)",NULL);
         hash_screen_640_x_200_monochrome_graphics  = hash("640x200 monochrome (graphics)",NULL);
         hash_screen_linewrap                       = hash("linewrap",NULL);
         hash_screen_320_x_200_color_graphics       = hash("320x200 color (graphics)",NULL);
         hash_screen_640_x_350_2_color_graphics     = hash("640x350 monochrome (2-color graphics)",NULL);
         hash_screen_640_x_350_16_color_graphics    = hash("640x350 color (16-color graphics)",NULL);
         hash_screen_640_x_480_2_color_graphics     = hash("640x480 monochrome (2-color graphics)",NULL);
         hash_screen_640_x_480_16_color_graphics    = hash("640x480 color (16-color graphics)",NULL);
         hash_screen_320_x_200_256_color_graphics   = hash("320x200 color (graphics)",NULL);
         hash_screen_restore                        = hash("restore",NULL);
         hash_screen_save                           = hash("save",NULL);
         hashes_initialized = true;
       }

       // log_info("key_hash: %lu, should be %lu\n", key_hash, hash_screen_40_x_25_monochrome_text);

       if HASH_CHECK(hash_screen_40_x_25_monochrome_text)       return (struct ansii_t){'=',.value.single=0,.suffix_ch='h',.valid=true}; // 40 x 25 monochrome (text)
       if HASH_CHECK(hash_screen_40_x_25_monochrome_text)       return (struct ansii_t){'=',.value.single=1,.suffix_ch='h',.valid=true}; // 40 x 25 color (text)
       if HASH_CHECK(hash_screen_40_x_25_color_text)            return (struct ansii_t){'=',.value.single=2,.suffix_ch='h',.valid=true}; // 80 x 25 monochrome (text)
       if HASH_CHECK(hash_screen_80_x_25_monochrome_text)       return (struct ansii_t){'=',.value.single=3,.suffix_ch='h',.valid=true}; // 80 x 25 color (text)
       if HASH_CHECK(hash_screen_80_x_25_color_text)            return (struct ansii_t){'=',.value.single=4,.suffix_ch='h',.valid=true}; // 320 x 200 4-color (graphics)
       if HASH_CHECK(hash_screen_320_x_200_4_color_graphics)    return (struct ansii_t){'=',.value.single=5,.suffix_ch='h',.valid=true}; // 320 x 200 monochrome (graphics)
       if HASH_CHECK(hash_screen_320_x_200_monochrome_graphics) return (struct ansii_t){'=',.value.single=6,.suffix_ch='h',.valid=true}; // 640 x 200 monochrome (graphics)
       if HASH_CHECK(hash_screen_640_x_200_monochrome_graphics) return (struct ansii_t){'=',.value.single=7,.suffix_ch='h',.valid=true}; // linewrap
       if HASH_CHECK(hash_screen_linewrap)                      return (struct ansii_t){'=',.value.single=13,.suffix_ch='h',.valid=true}; //320 x 200 color (graphics)
       if HASH_CHECK(hash_screen_320_x_200_color_graphics)      return (struct ansii_t){'=',.value.single=14,.suffix_ch='h',.valid=true}; //640 x 200 color (16-color graphics)
       if HASH_CHECK(hash_screen_640_x_350_2_color_graphics)    return (struct ansii_t){'=',.value.single=15,.suffix_ch='h',.valid=true}; //640 x 350 monochrome (2-color graphics)
       if HASH_CHECK(hash_screen_640_x_350_16_color_graphics)   return (struct ansii_t){'=',.value.single=16,.suffix_ch='h',.valid=true}; //640 x 350 color (16-color graphics)
       if HASH_CHECK(hash_screen_640_x_480_2_color_graphics)    return (struct ansii_t){'=',.value.single=17,.suffix_ch='h',.valid=true}; //640 x 480 monochrome (2-color graphics)
       if HASH_CHECK(hash_screen_640_x_480_16_color_graphics)   return (struct ansii_t){'=',.value.single=18,.suffix_ch='h',.valid=true}; //640 x 480 color (16-color graphics)
       if HASH_CHECK(hash_screen_320_x_200_256_color_graphics)  return (struct ansii_t){'=',.value.single=19,.suffix_ch='h',.valid=true}; //320 x 200 color (256-color graphics)
       if HASH_CHECK(hash_screen_restore)                       return (struct ansii_t){'?',.value.single=47,.suffix_ch='l',.valid=true}; //restore
       if HASH_CHECK(hash_screen_save)                          return (struct ansii_t){'?',.value.single=47,.suffix_ch='h',.valid=true}; //restore
       // case : return (struct ansii_t){'?',47,'h'}; //save
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}



unsigned int
fntattr_to_ansii(char* font_attr,struct hash* phash){
       unsigned long key_hash = hash(font_attr,phash);
       static bool hashes_initialized = false;
       static unsigned long ansii_hash_fontattr_bold, ansii_hash_fontattr_faint, ansii_hash_fontattr_italic, ansii_hash_fontattr_underline, ansii_hash_fontattr_blink, ansii_hash_fontattr_inverse, ansii_hash_fontattr_invisible, ansii_hash_fontattr_strikethrough;
       unsigned int attr_num;

       if(!hashes_initialized){
         log_info("initializing hashes\n");
         ansii_hash_fontattr_bold     = hash("bold",NULL);
         ansii_hash_fontattr_faint    = hash("faint",NULL);
         ansii_hash_fontattr_italic        = hash("italic",NULL);
         ansii_hash_fontattr_underline     = hash("underline",NULL);
         ansii_hash_fontattr_blink    = hash("blink",NULL);
         ansii_hash_fontattr_inverse       = hash("inverse",NULL);
         ansii_hash_fontattr_invisible     = hash("invisible",NULL);
         ansii_hash_fontattr_strikethrough = hash("strikethrough",NULL);
         hashes_initialized = true;
       }

       if HASH_CHECK(ansii_hash_fontattr_bold){                 attr_num = 1u;
       } else if HASH_CHECK(ansii_hash_fontattr_faint){         attr_num = 2u;
       } else if HASH_CHECK(ansii_hash_fontattr_italic){        attr_num = 3u;
       } else if HASH_CHECK(ansii_hash_fontattr_underline){     attr_num = 4u;
       } else if HASH_CHECK(ansii_hash_fontattr_blink){         attr_num = 5u;
       } else if HASH_CHECK(ansii_hash_fontattr_inverse){       attr_num = 6u;
       } else if HASH_CHECK(ansii_hash_fontattr_invisible){     attr_num = 7u;
       } else if HASH_CHECK(ansii_hash_fontattr_strikethrough){ attr_num = 8u;
       } else {
              log_error("Invalid font attribute: `%u`\n", font_attr);
              return 0; // reset
       }
       // log_info("(got) attribute: %u\n",attr_num);
       return attr_num;
}
struct ansii_t
color_to_ansii(char* color, struct hash* phash) {
       unsigned long key_hash = hash(color,phash);
       static bool hashes_initialized = false;
       static unsigned long hash_color_black, hash_color_red, hash_color_green, hash_color_yellow, hash_color_blue, hash_color_magenta, hash_color_cyan, hash_color_white;
       unsigned int r,g,b,id;
       struct ansii_t result = (struct ansii_t){.value_type=VALUE_SINGLE,.suffix_ch='m',.valid=true};

       if(!hashes_initialized){
         log_info("initializing hashes\n");
         hash_color_black = hash("black",NULL);
         hash_color_red   = hash("red",NULL);
         hash_color_green = hash("green",NULL);
         hash_color_yellow= hash("yellow",NULL);
         hash_color_blue  = hash("blue",NULL);
         hash_color_magenta = hash("magenta",NULL);
         hash_color_cyan  = hash("cyan",NULL);
         hash_color_white = hash("white",NULL);
         hashes_initialized = true;
       }
       if HASH_CHECK(hash_color_black){          result.value.single=0;
       } else if HASH_CHECK(hash_color_red){     result.value.single=1;
       } else if HASH_CHECK(hash_color_green){   result.value.single=2;
       } else if HASH_CHECK(hash_color_yellow){  result.value.single=3;
       } else if HASH_CHECK(hash_color_blue){    result.value.single=4;
       } else if HASH_CHECK(hash_color_magenta){ result.value.single=5;
       } else if HASH_CHECK(hash_color_cyan){    result.value.single=6;
       } else if HASH_CHECK(hash_color_white){   result.value.single=7;
       } else if(sscanf(color,"id-%u",&id) == 1){
              result.value.triplet[0] = 8;
              result.value.triplet[1] = 5;
              result.value.triplet[2] = id;
              result.value_type = VALUE_TRIPLET;
       } else if(sscanf(color,"%u-%u-%u",&r,&g,&b) == 3){
              result.value.quintet[0] = 8;
              result.value.quintet[1] = 2;
              result.value.quintet[2] = r;
              result.value.quintet[3] = g;
              result.value.quintet[4] = b;
              result.value_type = VALUE_QUINTET;
       } else {
              result.valid = false;
       }
       log_verbose("type: %d, value.single=%lu\n",result.value_type,result.value.single);
       return result;
}

struct ansii_t
parse_ansii_type(enum ansii_types ansii_type, char* value, struct hash* valhash){
       struct ansii_t ansii_r = {.value_type = VALUE_SINGLE};
       switch(ansii_type){
         case BG:
           ansii_r = color_to_ansii(value,valhash);
           if(ansii_r.value_type == VALUE_SINGLE){
                  ansii_r.value.single += 40;
           } else if (ansii_r.value_type == VALUE_QUINTET){ // rgb values
                  ansii_r.value.quintet[0] += 40;
           }
           break;
         case FG:
           ansii_r = color_to_ansii(value,valhash);
           if(ansii_r.value_type == VALUE_SINGLE){
                  ansii_r.value.single += 30;
           } else if (ansii_r.value_type == VALUE_QUINTET){ // rgb values
                  ansii_r.value.quintet[0] += 30;
           }
           break;
         case FONT_SET:
           ansii_r.value.single = fntattr_to_ansii(value,valhash);
           ansii_r.suffix_ch = 'm';
           ansii_r.valid = true;
           break;
         case FONT_RESET:
           ansii_r.value.single = 20 + fntattr_to_ansii(value,valhash);
           ansii_r.valid = true;
           ansii_r.suffix_ch = 'm';
           break;
           // more complicated cases that, aren't just based on value.
         case SCREEN: return scattr_to_ansii(value,valhash);
         case CURSOR: return cursor_to_ansii(value,valhash);
         case ALTBUF: return altbuf_to_ansii(value,valhash);
         case ERASE: return erase_to_ansii(value,valhash);
         case RESET:
           ansii_r.value.single = 0;
           ansii_r.valid = true;
           break;

       }
       return ansii_r;
}
int 
ansii_transform(FILE* from, FILE* to){
       // this assumes file `from` is readable, and `to`
       // is writable, and not equal. Make sure of this before calling this function.
#define BUFSIZE 1024
       char buf[BUFSIZE];
       char* save_ptr_vkey;

       enum ansii_types ansii_type;
       unsigned int read;
       struct ansii_t ansii_result;
       char* token_complete;
       char* end;
       char string_to_output[8];
       static bool initialized_hashes = false;
       unsigned int hash_bg, hash_fg, hash_font_set, hash_font_reset, hash_screen, hash_cursor, 
          hash_altbuf, hash_erase, hash_reset;
       unsigned int key_hash;
       struct hash khash = {};
       struct hash vhash = {};

       if(!initialized_hashes){
         hash_bg         = hash("bg",NULL);
         hash_fg         = hash("fg",NULL);
         hash_font_set   = hash("font-set",NULL);
         hash_font_reset = hash("font-reset",NULL);
         hash_screen     = hash("screen",NULL);
         hash_cursor     = hash("cursor",NULL);
         hash_altbuf     = hash("altbuf",NULL);
         hash_erase      = hash("erase",NULL);
         hash_reset      = hash("reset",NULL);
         
         initialized_hashes = true;
       }

       while((read = fread(buf,sizeof(char),BUFSIZE,from))){
         // char* token_complete = strtok_r(buf,"{}",&save_ptr_buf);
         log_verbose("buffer read %u characters\n",read);
         token_complete = buf;
         while((token_complete = strstr(token_complete, "{{")) != NULL){
           token_complete += 2;
           end = strstr(token_complete,"}}");
           if(!end){
             break;
           }
           *end = 0;
           char* key = strtok_r(token_complete,":",&save_ptr_vkey);
           // reset and redo hash for new key
           khash.computed = false;
           key_hash = hash(key,&khash);

           if HASH_CHECK(hash_bg){           ansii_type = BG;
           } else if HASH_CHECK(hash_fg){    ansii_type = FG;
           } else if HASH_CHECK(hash_font_set){   ansii_type = FONT_SET;
           } else if HASH_CHECK(hash_font_reset){ ansii_type = FONT_RESET; 
           } else if HASH_CHECK(hash_screen){     ansii_type = SCREEN; 
           } else if HASH_CHECK(hash_cursor){     ansii_type = CURSOR; 
           } else if HASH_CHECK(hash_altbuf){     ansii_type = ALTBUF;
           } else if HASH_CHECK(hash_erase){      ansii_type = ERASE;
           } else if HASH_CHECK(hash_reset){      ansii_type = RESET;
           }
           char* value = strtok_r(NULL,":",&save_ptr_vkey);
           vhash.computed = false;
           ansii_result = parse_ansii_type(ansii_type,value,&vhash);

           if(ansii_result.valid){
             switch(ansii_result.value_type){
                case VALUE_SINGLE:
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%d%c", ansii_result.prefix_ch,
                        ansii_result.value.single,
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%d%c",
                        ansii_result.value.single,
                        ansii_result.suffix_ch);
                  }
                  break;

                case VALUE_PAIR:
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%u;%u%c", ansii_result.prefix_ch, 
                        ansii_result.value.pair[0], ansii_result.value.pair[1],
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%u;%u%c",
                        ansii_result.value.pair[0], ansii_result.value.pair[1],
                        ansii_result.suffix_ch);
                  }
                  break;
                case VALUE_TRIPLET:
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%u;%u;%u%c", ansii_result.prefix_ch, 
                        ansii_result.value.triplet[0], ansii_result.value.triplet[1], ansii_result.value.triplet[2], 
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%u;%u;%u%c",
                        ansii_result.value.triplet[0], ansii_result.value.triplet[1], ansii_result.value.triplet[2],
                        ansii_result.suffix_ch);
                  }
                  break;
                // NTS: currently only used for fg/bg RGB values
                case VALUE_QUINTET: 
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%u;%u;%u;%u;%u%c", ansii_result.prefix_ch, 
                        ansii_result.value.quintet[0], ansii_result.value.quintet[1], ansii_result.value.quintet[2], ansii_result.value.quintet[3], ansii_result.value.quintet[4],
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%u;%u;%u;%u;%u%c",
                        ansii_result.value.triplet[0], ansii_result.value.triplet[1], ansii_result.value.triplet[2],ansii_result.value.quintet[3], ansii_result.value.quintet[4],
                        ansii_result.suffix_ch);
                  }
                  break;

                default:
                  break;
             }
           }
           // go until { and take the token
           token_complete = end + 2;

           end = strstr(token_complete,"{{");
           if(!end){
             break;
           }
           *end = '\0';
           fprintf(to,"%s",token_complete); // send to file the string until {{
           *end = '{';                 // reset the character
           fflush(to);
         }
       }
       return 0;

#undef BUFSIZE
}


