//screen:11*11 blocks
var jdw_con;
var jdw_elm;
var jdw_camera={"x":0,"y":0};

function jdw_init(){
  jdw_elm=document.getElementById("mycanvas");
  jdw_con=jdw_elm.getContext('2d');
}
function jdw_block_scr(x,y,c){
  jdw_con.fillStyle=c;
  jdw_con.strokeStyle = "black";
  jdw_con.lineWidth = 1;
  jdw_con.fillRect  (x-5,y-5,x+5,y+5);
  jdw_con.strokeRect(x-5,y-5,x+5,y+5);
}
function jdw_player_scr(x,y,f,c){
  jdw_con.fillStyle=c;
  jdw_con.strokeStyle = "black";
  jdw_con.lineWidth = 1;
  jdw_con.arc(x,y,9,0, Math.PI*2, true);
  jdw_con.lineWidth = 1;
  jdw_con.moveTo(x,y);
  if(f==0){
    jdw_con.lineTo(x+5,y);
  }else
  if(f==1){
    jdw_con.lineTo(x,y+5);  
  }else
  if(f==2){
    jdw_con.lineTo(x-5,y);
  }else
  if(f==3){
    jdw_con.lineTo(x,y-5);
  }
}

function jdw_abs2scr(x,y){
  var res={};
  var tx=(jdw_camera.x-6);
  var ty=(jdw_camera.y-6);
  res.x=(x-tx)*10;
  res.y=(y-ty)*10;
  return res;
}

function jdw_block_abs(x,y,c){
  var p=jdw_abs2scr(x,y);
  jdw_block_scr(p.x,p.y,c);
}
function jdw_getposi_time(bx,by,f,t){
  var x=bx;
  var y=by;
  
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
  return {"x":x,"y":y};
}

function jdw_carema_update(){
  var n=jubk_me[0];
  var x=jubk_me[1];
  var y=jubk_me[2];
  try{
    var p=jubk_player[n];
    if(p){
      var f=p[3];
      var t=p[4];
    }
    var pt=jdw_getposi_time(x,y,f,t);
    
    jdw_camera.x=pt.x;
    jdw_camera.y=pt.y;
    
  }catch(e){}
}
function jdw_obj_abs(x,y,obj){
  var sposi;
}
function jdw_player_abs(bx,by,f,c,t){
  var pt=jdw_getposi_time(bx,by,f,t);
  var p=jdw_abs2scr(pt.x,pt.y);
  jdw_player_scr(p.x,p.y,f,c);
}

function jdw_all_block(){
  var cx=Math.floor(jdw_camera.x);
  var cy=Math.floor(jdw_camera.y);
  var bx=cx-6;
  var by=cy-6;
  var ex=cx+6;
  var ey=cy+6;
  for(var x=bx;x<ex;x++){
    for(var y=by;y<ey;y++){
      
      if(x<0)continue;
      if(y<0)continue;
      if(jubk_map_size.x<x)continue;
      if(jubk_map_size.y<y)continue;
      
      try{
        var bk=jubk_map[x][y];
      }catch(e){
        continue;
      }
      var owner=bk[0];
      if(owner){
        try{
          var pl=jubk_player[owner];
          if(pl){
            jdw_block_abs(x,y,pl[2]);
          }
        }catch(e){}
      }
      
      var player=bk[2];
      if(player){
        try{
          var pt=jubk_player[player];
          if(pt){
            jdw_player_abs(x,y,pt[3],pt[2],jubk_time()-pt[4]);
          }
        }catch(e){}
      }
      var obj=bk[1];
      if(obj!=0){
        try{
          jdw_obj_abs(x,y,obj);
        }catch(e){}
      }
    }
  }
}

function jdw_render(){
  jdw_con.clearRect(0,0,jdw_elm.width,jdw_elm.height);
  jdw_carema_update();
  jdw_all_block();
  try{
    window.requestAnimationFrame(jdw_render);
  }catch(e){
    setTimeout(jdw_render,100);
  }
}
