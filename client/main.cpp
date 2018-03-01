#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/file.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;
int connfd;
bool gameover=false;
struct vec{
  int x;
  int y;
};
typedef Uint32 Color;
unsigned long long gettm(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
}
namespace draw{
  SDL_Window *Window = NULL;
  SDL_Surface *WindowScreen = NULL;
  SDL_Renderer *renderer = NULL;
  vector<SDL_Texture*> textures;
}

namespace game{
  int hp,pw;
  vec map_size;
  class Me{
    public:
  	 string name;
  	 int pw,hp;
  	 vec position;
  }me;

  class Player{
    public:
    vec position;          //position
    Color color;           //color
    int faceto;            //face to
    unsigned long long tm; //last update time
    void init(){
      position.x=0;
      position.y=0;
      faceto=0;
      tm=gettm();
	  color = SDL_MapRGB(draw::WindowScreen->format, (rand()%256), (rand()%256), (rand()%256));
    }
	void settm(){
      tm=gettm();
    }
  };
  unordered_map<string,Player> player;

  struct block{
    string player,owner;
    int obj;
	Color color_cache;
  };

  block ** gmap=NULL;

  void destroy(){
    if(gmap){
      for(int ix=0;ix<map_size.x;ix++)
        delete [] gmap[ix];
      delete [] gmap;
    }
  }
  void init(int x,int y){
	if(gmap)return;
    int ix,iy;
    gmap=new block*[x];
    for(ix=0;ix<x;ix++){
    gmap[ix]=new block[y];
      for(iy=0;iy<y;iy++){
        gmap[ix][iy].player.clear();
        gmap[ix][iy].owner.clear();
        gmap[ix][iy].obj=0;
      }
    }
  }

  inline void setname(const string & n){
    me.name=n;
  }

  inline int set_hp(int v){
    hp=v;
  }

  inline int set_pw(int v){
  	 pw=v;
  }

  inline void setme(int v1,int v2,int v3,int v4){
    me.position.x=v1;
    me.position.y=v2;
    set_hp(v3);
    set_pw(v4);
  }

  inline void send(const string & msg){
    const char * buf=msg.c_str();
    ::send(connfd,buf,strlen(buf),0);
    char end='\n';
    ::send(connfd,&end,sizeof(end),0);
  }

  inline void addplayer(const string & unm){
    player[unm].init();
  }

  inline void face(const string & unm,int fc){
    player[unm].faceto=fc;
  }

  inline void quit(){
    static const string str="quit";
    send(str);
  }

  inline void quitplayer(const string & unm){
    vec & posi=player[unm].position;
    int x=posi.x;
    int y=posi.y;
    gmap[x][y].owner.clear();
    player.erase(unm);
  }

  inline void createmap(int x,int y){
    map_size.x=x;
    map_size.y=y;
    init(x,y);
  }

  inline void setmapown(int x,int y,const string & o){
    if(x<0)return;
    if(y<0)return;
    if(x>map_size.x)return;
    if(y>map_size.y)return;
    gmap[x][y].owner=o;
  }

  inline void pick(int x,int y){
    if(x<0)return;
    if(y<0)return;
    if(x>map_size.x)return;
    if(y>map_size.y)return;
    gmap[x][y].obj=0;
  }

  inline void setmapobj(int x,int y,int o){
    if(x<0)return;
    if(y<0)return;
    if(x>map_size.x)return;
    if(y>map_size.y)return;
    gmap[x][y].obj=o;
  }

  inline void moveplayerto(const string & name,int nx,int ny){
    if(nx<0)return;
    if(ny<0)return;
    if(nx>=map_size.x)return;
    if(ny>=map_size.y)return;

    auto it=player.find(name);
    if(it==player.end())return;
    int x=it->second.position.x;
    int y=it->second.position.y;

    if(x<map_size.x)
    if(y<map_size.y)
    if(x>=0 && y>=0){
      block & ob=gmap[x][y];
      ob.player.clear();
    }

    it->second.position.x=x;
    it->second.position.y=y;
    it->second.tm=gettm();

    block & obp=gmap[x][y];
    obp.player=name;
    obp.owner =name;
	obp.color_cache=it->second.color;
  }

  int jubk_lastf;
  inline void walk(int f){
    if(f==jubk_lastf)return;
    jubk_lastf=f;
    char buf[64];
    snprintf(buf,64,"walk %d",f);
    send(buf);
  }

  inline void jubk_put(int i){
    char buf[64];
    snprintf(buf,64,"put %d",i);
    send(buf);
  }

  void onmsg(const string & m){
    istringstream iss(m);
    string method,name;
    int i1,i2,i3,i4;
    iss>>method;

    if(method=="addplayer"){
    	iss>>name;
      addplayer(name);
    }
    if(method=="cremap"){
    	iss>>i1;
    	iss>>i2;
      createmap(
        i1,i2
      );
    }else
    if(method=="quit"){
    	iss>>name;
      quitplayer(name);
    }else
    if(method=="move"){
    	iss>>name;
    	iss>>i1;
    	iss>>i2;
      moveplayerto(
        name,
        i1,i2
      );
    }else
    if(method=="face"){
    	iss>>name;
    	iss>>i1;
      face(name,i1);
    }else
    if(method=="setme"){
    	iss>>i1;
    	iss>>i2;
    	iss>>i3;
    	iss>>i4;
      setme(
        i1,i2,i3,i4
      );
    }else
    if(method=="pick"){
    	iss>>i1;
    	iss>>i2;
      pick(
        i1,i2
      );
    }else
    if(method=="setobj"){
    	iss>>i1;
    	iss>>i2;
    	iss>>i3;
      setmapobj(
        i1,i2,i3
      );
    }else
    if(method=="setown"){
    	iss>>i1;
    	iss>>i2;
    	iss>>name;
      setmapown(
        i1,i2,name
      );
    }else
    if(method=="setname"){
    	iss>>name;
      setname(name);
    }else
    if(method=="exit"){
      gameover=true;
    }
  }
}
namespace draw{
  struct{
    double x,y;
  }camera;
  
  inline void abs2scr(double x,double y,vec & res){
    double tx=(camera.x-2);
    double ty=(camera.y-2);
    res.x=floor((x-tx)*10);
    res.y=floor((y-ty)*10);
  }
  
  inline void getposi_time(double bx,double by,int f,double t,double * ret){
    double x=bx;
    double y=by;
  
    if(f==0){
      x+=(t-1);
    }else
    if(f==1){
      y+=(t-1);
    }else
    if(f==2){
      x-=(t-1);
    }else
    if(f==3){
      y-=(t-1);
    }
    ret[0]=x;
	ret[1]=y;
  }
  void loadTexture(const char * path){
    SDL_Surface *bitmapSurface = NULL;
    SDL_Texture *bitmapTex = NULL;
    bitmapSurface = SDL_LoadBMP(path);
	SDL_SetColorKey(bitmapSurface,true,SDL_MapRGB(bitmapSurface->format,255,255,255));
    bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
    SDL_FreeSurface(bitmapSurface);
    textures.push_back(bitmapTex);
  }

  void init(){
    SDL_Init(SDL_INIT_VIDEO);
    Window = SDL_CreateWindow(
      "jump block",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      500,                    500,
      SDL_WINDOW_SHOWN
    );
    if (Window == NULL){
      exit(1);
    }
    renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
  }

  void draw_texture(int id,int x,int y){
    if(id>textures.size())return;
	SDL_Rect sr,dr;
    if(x<0 || y<0){
	  int tx=x+10,
	      ty=y+10;
	  if(tx<0 || ty<0)return;
	  sr.x=0;
      sr.y=0;
	  sr.w=tx;
      sr.h=ty;
	}else{
      sr.x=x;
      sr.y=y;
	  sr.w=10;
      sr.h=10;
    }
    SDL_RenderCopy(renderer,textures[id],&sr,&dr);
  }

  inline void block_scr(int x,int y,Color c){
    
  }
  inline void block_abs(int x,int y,Color c){
    vec p;
    abs2scr(x,y,p);
    block_scr(p.x,p.y,c);
  }
  inline void obj_scr(int x,int y,int i){
    draw_texture(i+4,x,y);
  }
  inline void obj_abs(int x,int y,int i){
    vec p;
    abs2scr(x,y,p);
    obj_scr(p.x,p.y,i);
  }
  inline void player_scr(int x,int y,int f,Color c){
    if(f<4 && f>=0)draw_texture(f,x,y);
  }
  inline void player_abs(int x,int y,int f,Color c,double t){
    double pt[2];
    getposi_time(x,y,f,t,pt);
	vec p;
    abs2scr(pt[0],pt[1],p);
    player_scr(p.x,p.y,f,c);
  }
  inline void carema_update(){
    auto pl=game::player[game::me.name];
	double t=(pl.tm)/1000;
    double pt[2];
	getposi_time(pl.position.x,pl.position.y,pl.faceto,t,pt);
    
    camera.x=pt[0];
    camera.y=pt[1];
  }

  void all_block(){
    int cx=floor(camera.x);
    int cy=floor(camera.y);
	int bx=cx-2;
    int by=cy-2;
    int ex=cx+2;
    int ey=cy+2;
    for(int x=bx;x<ex;x++){
      for(int y=by;y<ey;y++){
      
        if(x<0)continue;
        if(y<0)continue;
        if(game::map_size.x<=x)continue;
        if(game::map_size.y<=y)continue;
      
        game::block & bk=game::gmap[x][y];
	    
        string & owner=bk.owner;
        
		block_abs(x,y,bk.color_cache);
		//if(!owner.empty()){
          //auto pl=game::player.find(owner);
          //if(pl!=game::player.end()){
          //  block_abs(x,y,pl->second.color);
          //}
        //}
      
        string & player=bk.player;
        if(!player.empty()){
          auto pt=game::player.find(player);
          if(pt!=game::player.end()){
            player_abs(
              x,y,
			  pt->second.faceto,pt->second.color,
              (gettm()-pt->second.tm)/1000
            );
          }
        }
        int obj=bk.obj;
        if(obj!=0){
          obj_abs(x,y,obj);
        }
      }
    }
  }
  void render(){
    SDL_RenderClear(renderer);
    
    carema_update();
    all_block();
    
    SDL_RenderPresent(renderer);
  }
}
int main(){
  SDL_Event e;
  draw::init();
  while(!gameover){
    draw::render();
	while( SDL_PollEvent( &e ) != 0 ){
      if( e.type == SDL_QUIT ){
        gameover=true;
      }
    }
  }
  SDL_FreeSurface(draw::WindowScreen);
  SDL_DestroyWindow(draw::Window);
  SDL_DestroyRenderer(draw::renderer);
  for(auto it:draw::textures){
    SDL_DestroyTexture(it);
  }
  SDL_Quit();
  return 0;
}
