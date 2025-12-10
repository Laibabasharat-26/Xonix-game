#include<SFML/Graphics.hpp>
#include<iostream>
#include<time.h>
#include<fstream>
#include<cmath>
using namespace sf;
using namespace std;

const int M=25;
const int N=40;

int grid[M][N]={0};  
const int ts=18; // tile size

// Game state variables
int game_phase=0; // 0: MENU, 1: LEVEL, 2: MODE, 3: scoreboard, 4: PLAY, 5: GAME OVER
int menuoption=0;
int lastEnemyAddTime = 0;

// Player 1 
int x=0;   
int y=0;   
int dx=0;  //horizontal movement
int dy=0;  //vertical movement
int score1=0;
int powerups1=0;
bool isbuilding1=false;
int tilescaptured1=0;
int movecounter1=0;
bool movepressed1=false;


// Player 2 
int x2=N-1;    
int y2=M-1;       
int dx2=0;
int dy2=0;
int score2=0;
int powerups2=0;
bool isbuilding2=false;
int tilescaptured2=0;
int movecounter2=0;
bool movepressed2=false;


bool Game=true;
bool istwoplayer=false;
int level=1;
int enemycount=2;
float timer=0;
float delay=0.07;
const float maxenemyspeed=3;
float enemySpeedMultiplier=1.0;
bool usepatterns=false;
int bonusthreshold=10;
int rewardcounter=0;


int highscore=0;
int topscores[5]={0};
int toptimes[5]={0};
bool isnewhighscore=false;

int lastDx=0;
int lastDy=0; 

struct enemy
{
    float x, y; 
    int dx, dy;
    enemy(){
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
        if (dx == 0) dx = 1;
        if (dy == 0) dy = 1;
    }
};

enemy a[10];
Clock gameclock;
Clock powerclock;
bool poweractive=false;
int ElapsedTime=0;

void drop(int y, int x) 
{
    if (grid[y][x] == 0) grid[y][x] = -1;
    if (grid[y-1][x] == 0) drop(y-1, x);
    if (grid[y+1][x] == 0) drop(y+1, x);
    if (grid[y][x-1] == 0) drop(y, x-1);
    if (grid[y][x+1] == 0) drop(y, x+1);
}

//for writing text on the board
Font font;
void drawText(RenderWindow& window,string str,int x,int y,int size,Color color = Color::White,bool boldtext=false) 
{
 Text text(str,font,size);
 text.setPosition(x,y);
 text.setFillColor(color);
if(boldtext)
{
 text.setStyle(Text::Bold);
}
 window.draw(text);
}

//displaying fist five high score 
void scoreboard(int topscores[],int toptimes[])
{
  ifstream file("scoreboard.txt");
  for(int i=0;i<5;i++)
  {
  if(!(file>>topscores[i]>>toptimes[i]))
  {
  topscores[i] = 0;
  toptimes[i] = 0;
  }
  }
file.close();
}

void savescoreboard()
{
ofstream fout("scoreboard.txt");
for(int i=0;i<5;i++)
{
  fout<<topscores[i]<<"\t"<<toptimes[i]<<endl;
}
fout.close();
}

void updatescoreboard(int score,int ElapsedTime)
{
    bool correctposition=false;
    for(int i=0;i<5;i++)
    {
      if(score>topscores[i])
      {
       for(int j=4;j>i;j--)
       {
          topscores[j]=topscores[j-1];
          toptimes[j]=toptimes[j-1];
       }
          topscores[i]=score;
          toptimes[i]=ElapsedTime;
          correctposition=true;
          break;
      }
    }    
    // Updating high score only if we got a new high score
    if(correctposition && score == topscores[0])
    {
      isnewhighscore = true;
      highscore = score; 
    }
    else
    {
      isnewhighscore = false;
    }
    savescoreboard();
}

void moveCircular(enemy &e,int i)
{
  float radius=50 +i*10;
  float angle=(gameclock.getElapsedTime().asSeconds()+i)*1.2;
  float newX=300+cos(angle)*radius;
  float newY=300+sin(angle)*radius;

  int gx=int(newX)/ts;
  int gy=int(newY)/ts;
  if(gx>=0 && gx<N && gy>=0 && gy<M && grid[gy][gx]!= 1 && grid[gy][gx]!= 2 && grid[gy][gx]!= 3) 
  {
   if(newX<0.0f)
   {
    e.x= 0.0f;
    }
   else if(newX>float(N*ts-1))
   {
    e.x=float(N*ts-1);
    }
   else
   {
    e.x = newX;
    }

   if(newY<0.0f)
   {
    e.y=0.0f;
    }
   else if(newY>float(M*ts-1))
   {
    e.y=float(M*ts-1);
    }
   else
   {
    e.y=newY;
    }
  }
}

void moveZigZag(enemy &e,int i) 
{
  float newX=e.x+e.dx*enemySpeedMultiplier;
  float newY=e.y+sin(newX*0.05)*2;

  int gx=int(newX)/ts;
  int gy=int(newY)/ts;

  if(gx>=0 && gx<N && gy>=0 && gy<M && grid[gy][gx]!=1 && grid[gy][gx]!=2 && grid[gy][gx]!=3)
  {
  if(newX<0.0f)
   {
    e.x= 0.0f;
    }
   else if(newX>float(N*ts-1))
    {
    e.x=float(N*ts-1);
    }
   else
    {
    e.x = newX;
    }

   if(newY<0.0f)
    {
    e.y=0.0f;
    }
   else if(newY>float(M*ts-1))
   {
    e.y=float(M*ts-1);
    }
   else
   {
    e.y=newY;
    }
    } 
  else 
  {
  e.dx=-e.dx;
  }
}

void moveLinear(enemy &e) 
{
  e.x+=e.dx *enemySpeedMultiplier;
  int gx=int(e.x)/ts;
  int gy=int(e.y)/ts;
  if(gx>=0 && gx<N && gy>=0 && gy<M && grid[gy][gx]==1)
  {
  e.dx=-e.dx;
  e.x+=e.dx *enemySpeedMultiplier;
  }
  e.y+=e.dy *enemySpeedMultiplier;
  gx=int(e.x)/ts;
  gy=int(e.y)/ts;
  if(gx>=0 && gx<N && gy>=0 && gy<M && grid[gy][gx]==1) 
  {
  e.dy=-e.dy;
  e.y+=e.dy*enemySpeedMultiplier;
  }
  if(e.x<0.0f)
  {
    e.x=0.0f;
  }
  else if(e.x>float(N*ts-1))
  {
    e.x=float(N*ts-1);
  }

  if(e.y<0.0f)
  {
    e.y=0.0f;
  }
  else if(e.y>float(M*ts-1))
  {
   e.y=float(M*ts-1);
  }

}

bool checkplayerenemycollision(int px,int py)
{
 for(int i=0;i<enemycount;i++) 
 {
 if(int(a[i].x)==px*ts && int(a[i].y)==py*ts) 
 {
 return true; 
 }
 }
 return false;
}

bool checkenemyonconstructingtile() 
{
  for (int i=0;i<enemycount;i++) 
  {
  //enemy positions
  int gx=int(a[i].x)/ts;
  int gy=int(a[i].y)/ts;
  if(grid[gy][gx]==2 || grid[gy][gx]==3) // Check for both players
  {  
  return true;  
  }
  }
  return false;
}

void moveenemy(enemy &e,int i)
{
 if(usepatterns)
 {
 if(i%2==0)
 {
 moveCircular(e, i);
 }
 else 
 {
 moveZigZag(e, i);
 }
 } 
 else
 {
 moveLinear(e);
 }
 if(checkplayerenemycollision(x,y)) 
 {
 Game=false;  
 }
 if(istwoplayer && checkplayerenemycollision(x2, y2)) 
 {
 Game=false;  
 }
 if (checkenemyonconstructingtile()) 
 {
 Game=false;  
 }
}

bool checkuturn(int lastDx,int lastDy,int newDx,int newDy) 
{
 if(grid[y2][x2]==2 || grid[y2][x2]==3)  //player 2
 {
 if(lastDx==-newDx && lastDy==-newDy)
 {
 return true; 
 }
 }
 if(grid[y][x]==2 || grid[y][x]==3)    //player 1
 {
 if(lastDx==-newDx && lastDy==-newDy)
 {
 return true; 
 }
 }
 if(grid[y][x]!=2 && grid[y][x]!=3)
 {  
 return false;  
 }
 return (lastDx==-newDx && lastDy==-newDy);
}

bool checkCollision(int x1,int y1,int x2,int y2) 
{
 return (x1==x2 && y1==y2);    //both players on the same tile
}

bool p1p2interaction() 
{
 if(istwoplayer) 
 {
 if(x==x2 && y==y2)   //players collided
 { 
 return true;
 }
 if(grid[y][x]==3 && isbuilding1)    //if Player 1 touches Player 2 constructing tile
 { 
 return true;
 }
 if(grid[y2][x2]==2 && isbuilding2)  //if Player 2 touches Player 1 constructing tile
 { 
 return true;
 }
 if(isbuilding1 && !isbuilding2 && checkCollision(x,y,x2,y2))  //If Player 1 collides with Player 2 while Player 2 is not constructing
 {  
 return true;
 }
 if(isbuilding2 && !isbuilding1 && checkCollision(x2, y2, x, y)) //If Player 2 collides with Player 1 while Player 1 is not constructing
 {  
 return true;
 }
 }
 return false;
}

int main()
{
 srand(time(0));
 RenderWindow window(VideoMode(N*ts,M*ts),"Xonix Game!");
 window.setFramerateLimit(60);

 font.loadFromFile("OpenSans-Regular.ttf");
 if(!font.loadFromFile("OpenSans-Regular.ttf")) 
 {
 cout<<"Failed to load font."<<endl;
 return 1;
 }

 Texture t1, t2, t3;
 t1.loadFromFile("images/tiles.png");
 t2.loadFromFile("images/gameover.png");
 t3.loadFromFile("images/enemy.png");
 
 scoreboard(topscores, toptimes);
 highscore = topscores[0]; 
    
 Sprite sTile(t1), senemy(t3);
 senemy.setOrigin(20, 20);

 for(int i=0;i<M;i++)
 {
 for(int j=0;j<N;j++)
 {
 if(i==0 || j==0 || i==M-1 || j==N-1) 
 {
 grid[i][j]=1;   //border
 }
 }
 }
    
 Clock clock;
 bool levelselected=false;
 bool modeselected=false;

 while(window.isOpen())
 {
 float time=clock.restart().asSeconds();
 clock.restart();
 timer+=time;

 Event e;
 while (window.pollEvent(e))
 {
 if(e.type==Event::Closed)
 window.close();

 if(e.type==Event::KeyPressed)
 {
 if(game_phase==4 && Game)    // Playing mode
 {
 if(e.key.code==Keyboard::Space && powerups1>0 && !poweractive)    // Player 1 power-up
 {
 powerups1--;
 poweractive=true;
 dx2=0;     //Player 2 freezes
 dy2=0;
 powerclock.restart();
 if(powerclock.getElapsedTime().asSeconds()>3) 
 { 
 poweractive=false;
 dx2+=x2;
 dy2+=y2;
 }
 }
                   
 if(istwoplayer && e.key.code==Keyboard::LShift && powerups2>0 && !poweractive)     // Player 2 power-up
 {
 powerups2--;
 poweractive = true;
 dx=0;
 dy=0;
 powerclock.restart();
 if(powerclock.getElapsedTime().asSeconds()>3) 
 { 
 poweractive=false;
 dx+=x;
 dy+=y;
 }
 }
 }

if(game_phase==0)   // Menu
{ 
 if(e.key.code==Keyboard::Up)
 {
 menuoption=(menuoption+3)%4;
 }
 if(e.key.code==Keyboard::Down) 
 {
 menuoption=(menuoption+1)%4;
 }
 if(e.key.code==Keyboard::Enter)
 {
 switch(menuoption)
 {
 case 0:
 game_phase=2;
 break;
 case 1:
 game_phase=1;
 break;
 case 2:
 game_phase=3;
 break;
 case 3:
 window.close();
 break;
 }
 }
 }
                 
 else if(game_phase==1)    // Level select
 { 
 if(e.key.code==Keyboard::Num1)
 {
 level=1; 
 levelselected=true;
 game_phase=0;
 }
 if(e.key.code==Keyboard::Num2)
 {
 level=2; 
 levelselected=true; 
 game_phase=0;
 }
 if(e.key.code==Keyboard::Num3)
 {
 level=3; 
 levelselected=true;
 game_phase=0;
 }
 if(e.key.code==Keyboard::Num4)
 {
 level=4;
 levelselected=true;
 game_phase=0;
 }
 if(e.key.code==Keyboard::Escape)
 {
 game_phase=0;
 }
 }
                
 else if(game_phase==2)    // Mode select
 { 
 if(e.key.code==Keyboard::Num1)
 {
 istwoplayer=false;
 modeselected=true;
 game_phase=0;
 }
 if(e.key.code==Keyboard::Num2)
 {
 istwoplayer=true;
 modeselected=true;
 game_phase=0;
 }
 if(e.key.code==Keyboard::Escape)
 {
 game_phase=0;
 }
 }
 else if(game_phase==5)    // Game over
 { 
 if(e.key.code==Keyboard::Num1)
 {
 levelselected=true;
 modeselected=true;
 game_phase=0;
 isbuilding1=false;
 isbuilding2=false;
 score1=0;
 score2=0;
 movecounter1=0;
 movecounter2=0;
 gameclock.restart();         
    ElapsedTime = 0;           
    timer = 0;                  
    rewardcounter = 0;          
    powerups1 = 0;             
    powerups2 = 0;              
    tilescaptured1 = 0;       
    tilescaptured2 = 0;
    poweractive = false;
 }
                    
 else if(e.key.code==Keyboard::Num2)
 { 
 game_phase=0; 
 gameclock.restart();         
  ElapsedTime = 0;           
  timer = 0;                  
  rewardcounter = 0;          
  powerups1 = 0;             
  powerups2 = 0;              
  tilescaptured1 = 0;       
  tilescaptured2 = 0;
  poweractive = false;
 }
 else if(e.key.code==Keyboard::Num3)
 {
 window.close();
 }
 }
 else if(game_phase==3)    // scoreboard
 { 
 if(e.key.code==Keyboard::Enter || e.key.code==Keyboard::Escape)
 {
 game_phase=0;
 }
 }
 }
 }

 if(levelselected && modeselected && game_phase==0)    //if these conditions are met game starts
 {
 Game=true;
 x=0;
 y=0;
 dx=0;
 dy=0;
 x2=N-1;
 y2=M-1;
 dx2=0;
 dy2=0;
 score1=0;
 score2=0;
 movecounter1=0;
 movecounter2=0;
 lastDx=0;
 lastDy=0;
 //easy,med, diff modes        
 switch(level)
 {
 case 1:
 enemycount=2;
 delay=0.07;
 break;
 case 2:
 enemycount=4;
 delay=0.06;
 break;
 case 3:
 enemycount=6;
 delay=0.05;
 break;
 case 4:
 enemycount=2;
 delay=0.07;
 lastEnemyAddTime=0;
 break;
 }
 enemySpeedMultiplier=1.0;
 usepatterns=false;
           
 //Clear grid
 for(int i=1;i<M-1;i++)
 {
 for(int j=1;j<N-1;j++)
 {
 grid[i][j]=0;
 }
 }
 // Reset enemies
 for(int i=0;i<enemycount;i++)
 {
 a[i]=enemy();
 game_phase=4;
 levelselected=false;
 modeselected=false;
 }
 }
if(game_phase==4 && Game) 
 {
    
   // Player 1 movement input
  if(Keyboard::isKeyPressed(Keyboard::Left)) 
  { 
  if(checkuturn(lastDx, lastDy, -1, 0) && !istwoplayer && isbuilding1)
  {
  Game=false;
  }
  dx=-1;
  dy=0;
  movepressed1 = true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::Right))
  { 
  if(checkuturn(lastDx, lastDy, 1, 0) && !istwoplayer && isbuilding1) 
  {
  Game=false;
  }
  dx=1; 
  dy=0; 
  movepressed1 = true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::Up))
  { 
  if(checkuturn(lastDx, lastDy, 0, -1) && !istwoplayer  && isbuilding1) 
  {
  Game=false;
  }
  dx=0; 
  dy=-1;
  movepressed1 = true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::Down))
  { 
  if(checkuturn(lastDx, lastDy, 0, 1) && !istwoplayer && isbuilding1) 
  {
  Game=false;
  }
  dx=0;
  dy=1;
  movepressed1 = true; 
  }
  if(checkplayerenemycollision(x, y))
  {
  Game = false; 
  }
  if(checkenemyonconstructingtile())
  {
  Game = false;  
  }
          
  if(istwoplayer)
  {
  if(Keyboard::isKeyPressed(Keyboard::S))
  { 
  if(checkuturn(lastDx, lastDy, -1, 0) && istwoplayer && isbuilding2) 
  {
  Game=false;
  }
  dx=-1;
  dy=0; 
  movepressed2 = true;
  }
  if(Keyboard::isKeyPressed(Keyboard::W))
  { 
  if(checkuturn(lastDx, lastDy, 1, 0) && istwoplayer && isbuilding2)
  {
  Game = false;
  }
  dx=1; 
  dy=0; 
  movepressed2 = true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::D))
  { 
  if(checkuturn(lastDx, lastDy, 0, -1) && istwoplayer && isbuilding2) 
  {
  Game=false;
  }
  dx=0; 
  dy=-1;
  movepressed2= true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::A))
  { 
  if(checkuturn(lastDx, lastDy, 0, 1) && istwoplayer && isbuilding2) 
  {
  Game=false;
  }
  dx=0; 
  dy=1;
  movepressed2=true; 
  }
  if(checkplayerenemycollision(x, y))
  {
  Game=false;  
  }
  if(checkenemyonconstructingtile())
  {
  Game=false;  
  }
  }
  if(istwoplayer)
  {
  if(Keyboard::isKeyPressed(Keyboard::Left))
  { 
  if(checkuturn(lastDx, lastDy, -1, 0) && istwoplayer && isbuilding1)
  {
  Game=false;
  }
  dx=-1;
  dy=0;
  movepressed1=true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::Right))
  { 
  if(checkuturn(lastDx, lastDy, 1, 0) && istwoplayer && isbuilding1) 
  {
  Game=false;
  }
  dx=1;
  dy=0; 
  movepressed1=true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::Up))
  { 
  if(checkuturn(lastDx, lastDy, 0, -1) && istwoplayer && isbuilding1) 
  {
  Game=false;
  }
  dx=0;
  dy=-1;
  movepressed1=true; 
  }
  if(Keyboard::isKeyPressed(Keyboard::Down))
  { 
  if(checkuturn(lastDx, lastDy, 0, 1) && istwoplayer && isbuilding1) 
  {
  Game=false;
  }
  dx=0; 
  dy=1;
  movepressed1=true; 
  }
            
  if(checkplayerenemycollision(x, y))
  {
  Game=false;  
  }
  if(checkenemyonconstructingtile()) 
  {
  Game=false;  
  }
  }

  // Player 2 movement input
  if(istwoplayer)
  {
    if(Keyboard::isKeyPressed(Keyboard::A)) 
    { 
    dx2=-1;
    dy2=0; 
    movepressed2=true; 
    }
    if(Keyboard::isKeyPressed(Keyboard::D))
    {
    dx2=1;
    dy2=0; 
    movepressed2=true; 
    }
    if(Keyboard::isKeyPressed(Keyboard::W)) 
    { 
    dx2=0;
    dy2=-1;
    movepressed2=true; 
    }
    if(Keyboard::isKeyPressed(Keyboard::S))
    { 
    dx2=0;
    dy2=1;
    movepressed2=true; 
    }
    if(checkplayerenemycollision(x, y))
    {
    Game=false;  
    }
    if (checkenemyonconstructingtile())
    {
    Game = false; 
    }
}

          
if(timer>delay)
{
lastDx=dx;
lastDy=dy;
// Player 1 movement
if(grid[y][x]==1 && movepressed1) 
{
x+=dx;   //tile by tile movement on built tiles
y+=dy;
movepressed1=false;
} 
else if(grid[y][x]!=1) 
{
x+=dx;  // continuous movement
y+=dy;
}

// Player 2 movement (two player mode)
if(istwoplayer)
{
if(grid[y2][x2]==1 && movepressed2) 
{
x2+=dx2;
y2+=dy2;
movepressed2=false;
} 
else if(grid[y2][x2]!=1) 
{
x2+=dx2;
y2+=dy2;
}
if(checkplayerenemycollision(x, y))
{
Game=false;
}
if(checkenemyonconstructingtile())
{
Game=false;
}
}
//boundary checks
if(x<0)
{
x=0;
}
else if(x>=N)
{
x=N-1;
}
if(y<0)
{
y=0;
}
else if(y>=M)
{
y=M-1;
}

if(istwoplayer) 
{
  if(x2<0)
  {
  x2=0;
  }
  else if(x2>=N)
  {
  x2=N-1;
  }
  if(y2<0)
  {
  y2=0;
  }
  else if(y>=M)
  {
  y2=M-1;
  }
}

// Player 1 tile building
if(grid[y][x]==0)
{
grid[y][x]=2; 
tilescaptured1++;
if(!isbuilding1) 
{
movecounter1++;
isbuilding1=true;
}
}
// Player 2 tile building 
if(istwoplayer && grid[y2][x2]==0) 
{
grid[y2][x2]=3; 
tilescaptured2++;
if(!isbuilding2)
{
movecounter2++;
isbuilding2=true;
}
}

//boundary check
if(x2<0)
{
 x2=0;
} 
else if(x2>=N)
{
  x2=N-1;
}
if(y2<0)
{
 y2=0;
}
else if(y2>=M)
{
y2=M-1;
}

// Check player interaction in two-player mode
if(p1p2interaction()) 
{
Game=false; 
}

//scoring system single player mode
if (grid[y][x]==1 && isbuilding1)
{
isbuilding1=false;
dx=dy=0;
  
if(tilescaptured1>bonusthreshold)
{
rewardcounter++;
if(rewardcounter>=5)
{
score1+=tilescaptured1*4;
} 
else
{
score1+=tilescaptured1*2;
}
if(rewardcounter==3) 
{
bonusthreshold=5;
}
} 
else 
{
score1+=tilescaptured1;
}
tilescaptured1=0;

// power-ups
if(score1==50 || score1==70 || score1==100 || score1==130|| (score2>130 && (score2-130)%30==0))
{
powerups1++;
}
                  
// Drop and fill
for(int i=0;i<enemycount;i++) 
drop(int(a[i].y)/ts, int(a[i].x)/ts);

 for(int i= 0;i<M;i++)
 {
   for(int j=0;j<N;j++)
   {
   grid[i][j]=(grid[i][j] == -1)? 0:1;
   }
   }
}

//rewarding system 
if(istwoplayer && grid[y2][x2]==1 && isbuilding2) 
{
// Stop building for Player 2
isbuilding2=false;
dx2=dy2=0;

if(tilescaptured2>bonusthreshold) 
{
rewardcounter++; 
if(rewardcounter>=5) 
{
score2+=tilescaptured2*4;  
}
else 
{
score2+=tilescaptured2*2;
}
} 
else 
{
score2+=tilescaptured2; 
}

tilescaptured2=0;  
if(istwoplayer && (score2==50 || score2==70 || score2==100 || score2==130 || (score2>130 && (score2-130)%30==0))) 
{
powerups2++;
}

// Adjust bonus thresholds based on occurrences
if(rewardcounter==3)
{
bonusthreshold=5; 
}
if(rewardcounter>= 5)
{
bonusthreshold=5; 
}
// Drop and fill logic for Player 2
for(int i=0;i<enemycount;i++)
{ 
drop(int(a[i].y)/ts, int(a[i].x)/ts);
}
for(int i=0;i<M;i++)
{
for(int j=0;j<N;j++)
{
grid[i][j]= (grid[i][j]==-1) ? 0 : 1;
}
}
}
if(isnewhighscore)
{
score1=highscore;
highscore=score1;
}
timer = 0;
}

// Move enemies only if power is not active
if(!poweractive)
{
for(int i=0;i<enemycount;i++) 
{
moveenemy(a[i],i);
}
}

// Updating game timer and difficulty
ElapsedTime = int(gameclock.getElapsedTime().asSeconds());
if(poweractive && powerclock.getElapsedTime().asSeconds()>3) 
{
poweractive = false;
}
if(ElapsedTime>0 && ElapsedTime%20== 0) 
{
enemySpeedMultiplier += 0.2;
if (enemySpeedMultiplier>maxenemyspeed)
{
    enemySpeedMultiplier = maxenemyspeed;
}
}
if(ElapsedTime>=30)
{
usepatterns=true;
}
//continuous mode enemy increase
if(level==4) 
{
  if(ElapsedTime-lastEnemyAddTime>=20 && enemycount<10)
    {
      enemycount+=2;
      if(enemycount>10)
      {
      enemycount = 10;
      }
      for(int i=0;i<enemycount;i++)
      {
          a[i]=enemy(); // reset enemies
      }
      lastEnemyAddTime= ElapsedTime;
    }
}

ElapsedTime= int(gameclock.getElapsedTime().asSeconds());
if(poweractive && powerclock.getElapsedTime().asSeconds() > 3) 
{
poweractive = false;
}

if(ElapsedTime>0 && ElapsedTime%20== 0) 
{
enemySpeedMultiplier= min(enemySpeedMultiplier + 0.2f,maxenemyspeed);
}
if(ElapsedTime>=30)
{
usepatterns=true;
}
}
 
 
// Rendering
window.clear();

// Draw grid
for(int i=0;i<M;i++)
{
for(int j=0;j<N;j++)
{
   if(grid[i][j]==0)
   {
   continue;
   }
   if(grid[i][j]==1)  // Border
   {
   sTile.setTextureRect(IntRect(0, 0, ts, ts)); 
   }
   if(grid[i][j]==2)   // Player 1 path
   {
   sTile.setTextureRect(IntRect(54, 0, ts, ts)); 
   } 
   if(grid[i][j]==3)    // Player 2 path
   {
   sTile.setTextureRect(IntRect(72, 0, ts, ts)); 
   }
   sTile.setPosition(j * ts, i * ts);
   window.draw(sTile);
   }
}

// Drawing players
sTile.setTextureRect(IntRect(36, 0, ts, ts));  // Player 1
sTile.setColor(Color::Yellow);
sTile.setPosition(x * ts, y * ts);
window.draw(sTile);

if(istwoplayer) 
{
sTile.setTextureRect(IntRect(107, 0, ts, ts)); // Player 2 
sTile.setColor(Color::Yellow); 
sTile.setPosition(x2 * ts, y2 * ts);
window.draw(sTile);
}

// Drawing enemies
senemy.rotate(10);
for (int i=0; i<enemycount;i++)
{
senemy.setPosition(a[i].x, a[i].y);
window.draw(senemy);
}

// Game over handling
if(!Game && game_phase==4)
{
if(istwoplayer) 
{
updatescoreboard(max(score1, score2), ElapsedTime);
} 
else
{
updatescoreboard(score1, ElapsedTime);
}
game_phase=5;
}


switch(game_phase)
{
  case 0: // Menu
  drawText(window,"XONIX GAME",230,65,40,Color::Blue,true);
  drawText(window,string(menuoption==0?"-- ":"  ")+"Start Game",267,150,24,Color::Red);
  drawText(window,string(menuoption==1?"-- ":"  ")+"Select Level",269,200,24,Color::Red);
  drawText(window,string(menuoption==2?"-- ":"  ")+"View scoreboard",240,250,24,Color::Red);
  drawText(window,string(menuoption==3?"-- ":"  ")+"Exit",310,300,24,Color::Red);
  break;
            
  case 1: // Level select
  drawText(window,">>>Select Level<<<",175,90,40,Color::Blue,true);
  drawText(window,"1. Easy",300,160,24,Color::Red);
  drawText(window,"2. Medium",300,200,24,Color::Red);
  drawText(window,"3. Hard",300,240,24,Color::Red);
  drawText(window,"4. Continuous",300,280,24,Color::Red);
  break; 
              
  case 2: // Mode select
  drawText(window,">>>Select Mode<<<",175,130,40,Color::Blue,true);
  drawText(window,"1. Single Player",275,180,24,Color::Red);
  drawText(window,"2. Two Player",275,220,24,Color::Red);
  break;
                
  case 3: // scoreboard
  drawText(window,">>>scoreboard<<<",210,100,28,Color::Blue,true);
  for(int i=0;i<5;i++)
  {
  string entry=to_string(i+1)+". Score: "+to_string(topscores[i])+"  Time: "+to_string(toptimes[i])+"s";
  drawText(window,entry,228,140+i*30,24,Color::Red);
  }
  break;
                
  case 4: // Gameplay
   if(istwoplayer)
   {
   drawText(window,"P1 Score: "+to_string(score1),17,10,24,Color::Green);
   drawText(window,"P1 Power-up: "+to_string(powerups1),17,40,24,Color::Magenta);
   drawText(window,"P2 Score: "+to_string(score2),520,10,24,Color(255, 165, 0)); // Orange
   drawText(window,"P2 Power-up: "+to_string(powerups2),520,40,24,Color::Cyan);
   drawText(window,"P1: Arrows, SPACE",17,70,20,Color::White);
   drawText(window,"P2: WASD, LSHIFT",520,70,20,Color::White);
   }
   else
   {
   drawText(window,"Score: "+to_string(score1),17,10,24,Color::Green);
   drawText(window,"Power-up: "+to_string(powerups1),17,40,24,Color::Magenta);
   drawText(window,"Press SPACE to use power-up!",17,70,20,Color::White);
   }
   drawText(window,"Moves: " + to_string(movecounter1),17,100,24,Color::Yellow);
   if(istwoplayer)
   {
   drawText(window,"P2 Moves: "+to_string(movecounter2),520,100,24,Color::Yellow);
   }
   drawText(window,"Time: "+to_string(ElapsedTime)+"s",299,20,24,Color::Cyan);
   break;
                
  case 5: // Game over
    drawText(window,"GAME OVER",180,80,60,Color::Blue,true);
    if(istwoplayer)
    {
        string winner;
        if(score1>score2)
        {
            winner="Player 1 Wins!";
        }
        else if(score2>score1)
        {
            winner="Player 2 Wins!";
        }
        else
        {
            winner="   It's a Tie!";
        }
        drawText(window,winner,250,160,40,Color::Yellow, true);
        drawText(window,"P1 Score: "+to_string(score1),190,239,24,Color::Green,true);
        drawText(window,"P2 Score: " +to_string(score2),390,239,24,Color(255,165,0),true);
        drawText(window, "High Score: " +to_string(topscores[0]),280,270,24,Color::Red,true);
         if(isnewhighscore)
    {
        drawText(window,"NEW HIGH SCORE!",200,400,32,Color::Yellow,true);
    }
    }
    else
    {
        drawText(window,"Score: "+to_string(score1),310,160,24,Color::Red,true);
        drawText(window,"High Score: "+to_string(topscores[0]),280,220,24,Color::Red,true);
    }
    if(!istwoplayer)
    {
    drawText(window," Moves: "+to_string(movecounter1),300,190,24,Color::Red,true);
    }
    if(istwoplayer)
    {
         drawText(window,"P1 Moves: "+to_string(movecounter1),190,215,24,Color::Red,true);
        drawText(window,"P2 Moves: "+to_string(movecounter2),390,215,24,Color::Red,true);
    }
    drawText(window,"1. Restart",110,320,24,Color::Red,true);
    drawText(window,"2. Menu",315,320,24,Color::Red,true);
    drawText(window,"3. Exit",490,320,24,Color::Red,true);
    if(isnewhighscore)
    {
        drawText(window,"NEW HIGH SCORE!",200,400,32,Color::Yellow,true);
    }
    break;
}

    window.display();
    }

    return 0;
}
