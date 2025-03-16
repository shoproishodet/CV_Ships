//======================================================================================
/*enum{
	K_ESC=256,K_TAB,K_RET,K_BACK,K_DEL,K_HOME,K_END,K_PGUP,K_PGDN,K_LEFT,K_RIGHT,K_UP,K_DOWN,
	K_SHIFT,K_CTRL,K_ALT,
	K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,K_F8,K_F9,K_F10,K_F11,K_F12
};*/
//======================================================================================
#define K_INS 1
#define K_DEL 2
#define K_HOME 3
#define K_END 4
#define K_PGUP 5
#define K_PGDN 6

#define K_BACK 8
#define K_TAB 9
#define K_SHIFT 10
#define K_CTRL 11
#define K_ALT 12
#define K_ENTER 13

#define K_PRIOR 14
#define K_NEXT 15
#define K_LEFT 16 
#define K_UP 17
#define K_RIGHT 18
#define K_DOWN 19

//#define K_MOUSE1 24
//#define K_MOUSE2 25
//#define K_MOUSE3 26

#define K_ESC 27
#define K_SPACE 32
//======================================================================================
int trans_sys_key(int i){
 switch(i){
  //case 1:return K_MOUSE1;
  //case 2:return K_MOUSE2;
  //case 3:return K_MOUSE3;
  case VK_INSERT:return K_INS;
  case VK_DELETE:return K_DEL;
  case VK_HOME:return K_HOME;
  case VK_END:return K_END;
//  case VK_PGUP:return K_PGUP;
//  case VK_PGDN:return K_PGDN;
  case VK_BACK:return K_BACK;
  case VK_TAB:return K_TAB;
  case 160:case 161:
  case VK_SHIFT:return K_SHIFT;
  case 163:case 162:
  case VK_CONTROL:return K_CTRL;
  case 164:case 165:
  case VK_MENU:return K_ALT;
  //case 91:case 92:win
  case VK_RETURN:return K_ENTER;
  case VK_PRIOR:return K_PRIOR;
  case VK_NEXT:return K_NEXT;
  case VK_LEFT:return K_LEFT;
  case VK_UP:return K_UP;
  case VK_RIGHT:return K_RIGHT;
  case VK_DOWN:return K_DOWN;
  case VK_ESCAPE:return K_ESC;
  case VK_SPACE:return K_SPACE;
  default:return i;
 };
};
//======================================================================================
int sys_spec(int i){
 switch(i){
  case 188:return ',';
  case 190:return '.';
  case 191:return '/';
  case 186:return ';';
  case 222:return 34;//'"';
  case 220:return 92;//'\';
  case 219:return '[';
  case 221:return ']';
  case 192:return '`';
  case 189:return '-';
  case 187:return '=';
  default:return i;
 };
};
//======================================================================================
int sys_shift_num(int i){
 switch(i){
  case '1':return '!';
  case '2':return '@';
  case '3':return '#';           
  case '4':return '$';
  case '5':return '%';
  case '6':return '^';
  case '7':return '&';
  case '8':return '*';
  case '9':return '(';
  case '0':return ')';

  case '`':return '~';
  case '-':return '_';
  case '=':return '+';
  case ',':return '<';
  case '.':return '>';
  case '/':return '?';
  case ';':return ':';
  case 34:return 39;//''' '"'; case '\'':ret='"';break;
  case 92:return '|';//'\'  case '\\':ret='|';break;
  case '[':return '{';
  case ']':return '}';

  default:return i;
 };
};
//======================================================================================
