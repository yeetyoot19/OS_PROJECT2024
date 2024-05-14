#include <iostream>
#include <SFML/Graphics.hpp>            
#include <pthread.h>
#include <time.h>
#include <cmath>
#include <semaphore.h>
#include <SFML/System/Clock.hpp>
#include <unistd.h>
using namespace std;
using namespace sf;

void SetPowerMode();

sem_t GhostKeys;
sem_t GhostPermits;
sem_t SpeedBoost;
pthread_mutex_t ReadWriteMutex;

float timer = 0;
float timing;

int maze[21][29] = 
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1},    
    {1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,1,1,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
sf::RectangleShape wall[250];

bool GameOpen = true;
int pacx;
int pacy;

float speedbst[4] = {0.5,0.5,0.5,0.5};


struct Pellets
{
    Sprite sprite;
    int x;
    int y;
    Texture tex;
    float disappear;
    float reappear;
    bool eaten;
    bool fruit;
    Pellets()
    {
        this->sprite.setPosition(0,0);
        this->tex.loadFromFile("pacman-art/other/dot.png");
        this->sprite.setTexture(this->tex);
        this->sprite.setScale(2,2);
        this->disappear = 0;
        this->reappear = 0;
        this->x = 0;
        this->y = 0;
        this->fruit = false;
        this->eaten = false;
    }
    Pellets(std::string img)
    {
        this->sprite.setPosition(0,0);
        this->tex.loadFromFile(img);
        this->sprite.setTexture(this->tex);
        this->sprite.setScale(2,2);
        this->disappear = 0;
        this->reappear = 0;
        this->x = 0;
        this->y = 0;
        this->fruit = false;
        this->eaten = false;
    }

};
Pellets Normal_Pellets[370];
Pellets Fruits[5]{
    Pellets("pacman-art/other/strawberry.png"),
    Pellets("pacman-art/other/strawberry.png"),
    Pellets("pacman-art/other/strawberry.png"),
    Pellets("pacman-art/other/strawberry.png"),
    Pellets("pacman-art/other/strawberry.png"),
};

pthread_mutex_t PlayerMutex;

struct Player
{
    Sprite sprite;
    float speed;
    Texture tex;
    int direction;
    int score;
    int Lives;
    bool PowerMode;
    float powStart;
    float powEnd;

    Player()
    {
        this->sprite.setPosition(200,40);
        this->tex.loadFromFile("pacman-art/pacman-right/2.png");
        this->sprite.setTexture(this->tex);
        this->sprite.setScale(1.2,1.2);
        this->speed = 0.9;
        this->direction = 1;
        this->score = 0;
        this->Lives = 3;
        pacx = 50;
        pacy = 50;
        this->PowerMode = false;
    }

	//readers
    float getTop()
    {    

        return this->sprite.getPosition().y - 8;

    }

    float getbottom()
    {

        return this->sprite.getPosition().y + 8;
        
   	}

    float getRight()
    {  
       
        return this->sprite.getPosition().x + 8;
       
               
    } 
    float getLeft()
    {    
      
        return this->sprite.getPosition().x - 8;
               
    }


    bool isWallCollision(int dx, int dy)
    {
        for( int i = 0 ; i < 250 ;i++){
        
            if(dx + 1  > (wall[i].getPosition().x - 20) &&
            dx - 1 < (wall[i].getPosition().x + 20) &&
            dy - 1 < (wall[i].getPosition().y + 20) &&
            dy + 1 > wall[i].getPosition().y - 20)
            {
                return 1;
            }
        }
        return 0;
    } 

	//writer
    void move()
    {
        pthread_mutex_lock(&PlayerMutex);
         
        if(direction == 1) // left
        {
            this->tex.loadFromFile("pacman-art/pacman-left/1.png");
            this->sprite.setTexture(this->tex);

            if( !isWallCollision(this->sprite.getPosition().x-1,this->sprite.getPosition().y) )
            {
                pthread_mutex_lock(&ReadWriteMutex);
                this->sprite.move(-1*speed,0);
                pthread_mutex_unlock(&ReadWriteMutex);
            }

        }
        else if(direction == 2)
        {
            this->tex.loadFromFile("pacman-art/pacman-right/1.png");
            this->sprite.setTexture(this->tex);

            if( !isWallCollision(this->sprite.getPosition().x+1,this->sprite.getPosition().y) )
            {
            	pthread_mutex_lock(&ReadWriteMutex);
                this->sprite.move(1*speed,0); // right
                pthread_mutex_unlock(&ReadWriteMutex);
            }
        }
        else if(direction == 3)
        {
            this->tex.loadFromFile("pacman-art/pacman-up/1.png");
            this->sprite.setTexture(this->tex);

            if( !isWallCollision(this->sprite.getPosition().x,this->sprite.getPosition().y-1) )
            {
            	pthread_mutex_lock(&ReadWriteMutex);
                this->sprite.move(0,-1*speed); // up
                pthread_mutex_unlock(&ReadWriteMutex);
            }

        }
        else if(direction == 4)
        {
        	
            this->tex.loadFromFile("pacman-art/pacman-down/1.png");
            this->sprite.setTexture(this->tex);

            if( !isWallCollision(this->sprite.getPosition().x,this->sprite.getPosition().y+1) )
            {
            	pthread_mutex_lock(&ReadWriteMutex);
                this->sprite.move(0,1*speed); // down
                pthread_mutex_unlock(&ReadWriteMutex);
            }

        }
        if(PelletPacCollision(Normal_Pellets,370))
        {
            this->score++;
            cout<<score<<endl;
        }
        if(FruitPacCollision(Fruits,5))
        {
            if(this->PowerMode){
                this->powEnd += 3; // powermode has been EXTENDED
            }
            else{
                this->PowerMode = true;
                this->powStart = timer;
                this->powEnd = powStart + 3;
                cout<<"power mode on! "<<this->powStart <<" ending: "<<this->powEnd<<endl;
                SetPowerMode();
            }
        }
        
        pacx = this->sprite.getPosition().x;
        pacy = this->sprite.getPosition().y;
        pthread_mutex_unlock(&PlayerMutex);
        
    }

    void GotUserKeyPress(sf::Keyboard::Key key) 
    {
        pthread_mutex_lock(&PlayerMutex);
        if (key == Keyboard::Left) 
            this->direction = 1;  
        if (key == Keyboard::Right) 
            this->direction = 2; 
        if (key == Keyboard::Up) 
            this->direction = 3; 
        if (key == Keyboard::Down) 
            this->direction = 4; 
        pthread_mutex_unlock(&PlayerMutex);
        
    }

	//reader
    bool PelletPacCollision(Pellets (NormalPellets)[370], int pelletCount)
    {

        for( int i = 0; i < pelletCount; i++)
        {
            
            if(this->sprite.getPosition().x > (NormalPellets[i].sprite.getPosition().x - 12) &&
            this->sprite.getPosition().x < (NormalPellets[i].sprite.getPosition().x + 12) &&
            this->sprite.getPosition().y  < (NormalPellets[i].sprite.getPosition().y + 12) &&
            this->sprite.getPosition().y  > NormalPellets[i].sprite.getPosition().y - 12)
            {

                Normal_Pellets[i].sprite.setPosition(660,660);
                Normal_Pellets[i].eaten = true;
                return 1;
            }

        }
        return 0;
    }

//reader
    bool FruitPacCollision(Pellets (Fruits)[5], int fruitCount)
    {

        for( int i = 0; i < fruitCount; i++)
        {
        	
            
            if(this->sprite.getPosition().x > (Fruits[i].sprite.getPosition().x - 12) &&
            this->sprite.getPosition().x < (Fruits[i].sprite.getPosition().x + 12) &&
            this->sprite.getPosition().y  < (Fruits[i].sprite.getPosition().y + 12) &&
            this->sprite.getPosition().y  > Fruits[i].sprite.getPosition().y - 12)
            {
               
                Fruits[i].eaten = true;
                Fruits[i].sprite.setPosition(660,660);
                Fruits[i].disappear = timer;
                Fruits[i].reappear = Fruits[i].disappear + 7;
                return 1;
            }

        }
        return 0;
    }

};

Player* P = new Player;

pthread_mutex_t GhostMutex[4];
struct Ghost
{
    Sprite sprite;
    float speed = 0.5;
    Texture tex;
    int direction;
    int x;
    int y;
    bool isMoving;
    bool hasBoost;
    Ghost(std::string img, int x, int y)
    {
        this->tex.loadFromFile(img);
        this->sprite.setPosition(x,y);
        this->sprite.setTexture(this->tex);
        this->sprite.setScale(1.5,1.5);
        this->direction = 0;
        this->speed = 1.1;
        this->x = x;
        this->y = y;
        this->isMoving = false;
        this->hasBoost = false;
    }
    

	bool GhostPacCollision(int dx, int dy)
    {   
        if(dx + 1  > (this->sprite.getPosition().x - 20) &&
        dx - 1 < (this->sprite.getPosition().x + 20) &&
        dy - 1 < (this->sprite.getPosition().y + 20) &&
        dy + 1 > this->sprite.getPosition().y - 20){
            return 1;
        }
        return 0;
    }

    void move(int index){
        if(this->direction == 1){ //left 
            this->sprite.move(-1*speed*speedbst[index],0);
        }
        if(this->direction == 2){ // right
            this->sprite.move(1*speed*speedbst[index],0);
        }
        if(this->direction == 3){ //up
            this->sprite.move(0,-1*speed*speedbst[index]);
        }
        if(this->direction == 4){ //down
            this->sprite.move(0,1*speed*speedbst[index]);
        }
    }
    
    //reader function
    void Directed_Movement_Ghost(int ghostIndex)
    {
    	if(ghostIndex == 0 || ghostIndex == 2)
    	{
    		if(this->sprite.getPosition().x == 220 && this->sprite.getPosition().y == 210)
    		{ // move right
                this->direction = 2; 
    		}
    		 
            else if(this->sprite.getPosition().x > 280 && this->sprite.getPosition().y == 210) { //move down
                this->direction = 4;
            }
    		
    		else if(this->sprite.getPosition().x > 125 && this->sprite.getPosition().y >= 270){ //move left
    			this->direction = 1;
    		}

            else if((this->sprite.getPosition().x < 125 && this->sprite.getPosition().x > 55) && (this->sprite.getPosition().y > 270 && this->sprite.getPosition().y < 370)){ // move down
                this->direction = 4;
            }

            else if((this->sprite.getPosition().x < 125 && this->sprite.getPosition().x > 55 ) && this->sprite.getPosition().y > 370 ){ // move left
                this->direction = 1; 
            }

            else if(this->sprite.getPosition().x < 55 && this->sprite.getPosition().y > 370){ // move up
                this->direction = 3; 
            }

            else if(this->sprite.getPosition().x < 55 && this->sprite.getPosition().y < 30){// move right 
                this->direction = 2;
            }

            else if(this->sprite.getPosition().x > 125 && this->sprite.getPosition().y < 30){ // move down
                this->direction = 4;
            }
           //cout<<this->sprite.getPosition().x<<" "<<this->sprite.getPosition().y<<endl;
    	}
    	else if(ghostIndex == 1 || ghostIndex == 3){
            this->speed = 1;
            if(this->sprite.getPosition().x == 250 && this->sprite.getPosition().y == 210){ // move right
                this->direction = 2;
            }
            else if(this->sprite.getPosition().x == 280 && this->sprite.getPosition().y == 210) { //move down
                this->direction = 4;
            }
    		else if(this->sprite.getPosition().x == 280 && this->sprite.getPosition().y == 270){ //move right
    			this->direction = 2;
    		}
            else if(this->sprite.getPosition().x == 510 && this->sprite.getPosition().y == 270){ // move up
                this->direction = 3;
            }
            else if(this->sprite.getPosition().x == 510 && this->sprite.getPosition().y == 185){ // move left
                this->direction = 1;
            }
            else if(this->sprite.getPosition().x == 425 && this->sprite.getPosition().y == 185){ // move up
                this->direction = 3;
            }
            else if(this->sprite.getPosition().x == 425 && this->sprite.getPosition().y == 110){ // move right
                this->direction = 2;
            }
            else if(this->sprite.getPosition().x == 510 && this->sprite.getPosition().y == 110){ // move up
                this->direction = 3;
            }
            else if(this->sprite.getPosition().x == 510 && this->sprite.getPosition().y == 30){ // move left
                this->direction = 1;
            }
            else if(this->sprite.getPosition().x == 423 && this->sprite.getPosition().y == 30){ // move down
                this->direction = 4;
            }
            else if(this->sprite.getPosition().x == 423 && this->sprite.getPosition().y == 270){ // move right
                this->direction = 2;
            }
    	}

        move(ghostIndex); // direction changed, now apply    

        pthread_mutex_lock(&ReadWriteMutex);
        if(GhostPacCollision(P->sprite.getPosition().x, P->sprite.getPosition().y) && !P->PowerMode){
        	
        	cout<<"Life lost"<<endl;
            P->Lives--;
        	P->sprite.setPosition(200,40);
        }
        else if(GhostPacCollision(P->sprite.getPosition().x, P->sprite.getPosition().y) && P->PowerMode)
        {
            
            this->isMoving = false;
            this->sprite.setPosition(this->x,this->y);
            
            sf::sleep(sf::milliseconds(100));
            this->HandleKeyPermitRe();
            sf::sleep(sf::milliseconds(100));
        }
        pthread_mutex_unlock(&ReadWriteMutex);
        
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(this->sprite);
    }
    bool HandleKeyPermit()
    {
        bool HaveKey = false; 
        bool HavePermit = false;
       
        HaveKey = !sem_trywait(&GhostKeys);

        HavePermit = !sem_trywait(&GhostPermits);

        if(HavePermit && HaveKey)
        {
            return true;
        }
        else if(HavePermit)
        {
            sem_post(&GhostPermits);
            return false;
        }
        else if(HaveKey)
        {
            sem_post(&GhostKeys);
            return false;
        }
        return false;
    }
    bool HandleBoost()
    {
    	bool HasBoost = false;
    	HasBoost = !sem_trywait(&SpeedBoost);
    	if(HasBoost)
    	{
    		return true;
    	}
    	return false;
    }

    void HandleKeyPermitRe()
    {
        sem_post(&GhostKeys);
        sem_post(&GhostPermits);
    }
    void HandleBoostRe()
    {
    	sem_post(&SpeedBoost);
    }
};

Ghost Ghosts[4]
{ 
	Ghost("pacman-art/ghosts/blinky.png", 220, 210),
    Ghost("pacman-art/ghosts/clyde.png", 250, 210), 
    Ghost("pacman-art/ghosts/inky.png", 310, 210),
    Ghost("pacman-art/ghosts/pinky.png", 280, 210)
};

void* Ghost_Movement(void* arg)
{
    int ghostIndex = *((int*)arg)-1;
	
    while (GameOpen) 
    {
    	pthread_mutex_lock(&GhostMutex[ghostIndex]);
		 cout<<"i am "<<ghostIndex<<" and i am checking"<<endl;
		if(Ghosts[ghostIndex].HandleKeyPermit())
		{
		    Ghosts[ghostIndex].isMoving = true;
		}
		if(Ghosts[ghostIndex].HandleBoost())
		{
			Ghosts[ghostIndex].hasBoost = true;
		}
		
		pthread_mutex_unlock(&GhostMutex[ghostIndex]);
		 if(Ghosts[ghostIndex].hasBoost == true)
        {
        	cout<<"I am "<<ghostIndex<<" and I have boost"<<endl;
        	speedbst[ghostIndex] = 1;
        }
        if(Ghosts[ghostIndex].isMoving == true)
        {
            cout<<"hello i am "<<ghostIndex<<" and i have the permit"<<endl;
            Ghosts[ghostIndex].Directed_Movement_Ghost(ghostIndex);
        }
        
       
        /*
        else
        {
        	speedbst[ghostIndex] = 0.5;
        	cout<<"I am "<<ghostIndex<<" and i do not have boost"<<endl;
        }
        */
        sf::sleep(sf::milliseconds(10));
    }
    pthread_exit(NULL);
}

void SetPowerMode()
{
    Texture* tex = new Texture;
    for(int i = 0 ; i < 4; i++){
        tex->loadFromFile("pacman-art/ghosts/blue_ghost.png");
        Ghosts[i].sprite.setTexture(*tex);
    }
}

void UnPowerMode(){
    if(P->PowerMode){
        if(P->powEnd <= timer ){
            for(int i = 0 ; i < 4; i++){
                Ghosts[i].sprite.setTexture(Ghosts[i].tex);
            }
            P->PowerMode = false;
            cout<<"removing power mode!"<<endl;
        }
    }
}

//Writer thread : writes pacman positions
void* updateFunction(void*)
{
    while(GameOpen)
    {
        P->move();
        sf::sleep(sf::milliseconds(10));
    }

    pthread_exit(NULL);
}

void reSpawnPellets(float& timer, int& fruitCount){

    for( int i = 0 ; i < fruitCount; i++){
        if(Fruits[i].eaten){
            if(Fruits[i].reappear <= timer){
                Fruits[i].sprite.setPosition(sf::Vector2f(Fruits[i].x,Fruits[i].y));
                Fruits[i].eaten = false;
            }
        }
    }
}

void SetMaze(sf::RenderWindow& window, int (maze)[21][29], sf::RectangleShape (wall)[])
{
    int count = 0;
    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 29; ++x) {
            if (maze[y][x] == 1) {
                wall[count++].setPosition(x * 20, y * 20);
            }
        }
    }
}

void DisplayMaze(sf::RenderWindow& window, int(maze)[21][29], sf::RectangleShape (wall)[])
{
    int count = 0;
    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 29; ++x) {
            if (maze[y][x] == 1) {
                window.draw(wall[count++]);
            }
        }
    }
}

void DisplayPellets(sf::RenderWindow& window, Pellets (Normal_Pellets)[], int& pelletCount)
{
    int count = 0;
    for( int i = 0; i < 21; i++){
        for(int j = 0 ; j < 29 ; j++){
            if(maze[i][j]==0){
                window.draw(Normal_Pellets[count++].sprite);
            }
        }
    }
    for(int i = 0; i< 5; i++){
        window.draw(Fruits[i].sprite);
    }
}

void DrawLife (sf::RenderWindow& window, Sprite(Lifes)[]){
    for(int i = 0 ; i < P->Lives; i++){
        Lifes[i].setPosition(400+i*40,440);
        Lifes[i].setScale(2,2);
        window.draw(Lifes[i]);
    }
}

void SetPellets(Pellets (Normal_Pellets)[],int& pelletCount, int& fruitCount){
    for( int i = 0; i < 21; i++){
        if(i==1 || i==19 || i == 18 || i==17) continue;
        for(int j = 0 ; j < 29 ; j++){
            if( ((i==10 || i == 11) || (i==13 || i==14)) && ((j == 4 || j == 5) ||(j == 23 || j == 24) ))
            continue; 
            if((i==13 || i == 14) && ( (j>=8 && j<=11) || (j>=17 && j<=20) ))
            continue;
            if(i==16 && j == 14)
            continue;
            if( (i>=8 && i<=11) && (j>=10 && j<=18))
            continue;
            if(maze[i][j]==0){
                Normal_Pellets[pelletCount].sprite.setPosition(sf::Vector2f((j * 20)-10,(i* 25)-35));
                Normal_Pellets[pelletCount].x = Normal_Pellets[pelletCount].sprite.getPosition().x;
                Normal_Pellets[pelletCount].y = Normal_Pellets[pelletCount].sprite.getPosition().y;                
                pelletCount++;
            }
        }
    }
    
    Fruits[0].sprite.setPosition(80,360);
    Fruits[0].x = 80; Fruits[0].y = 360;
    Fruits[1].sprite.setPosition(470,360);
    Fruits[1].x = 470; Fruits[1].y = 360;
    Fruits[2].sprite.setPosition(40,110);
    Fruits[2].x = 40; Fruits[2].y = 110;
    Fruits[3].sprite.setPosition(500,110);
    Fruits[3].x = 500; Fruits[3].y = 110;
    fruitCount = 5;
}


int main()
{ 
    //window
    bool CloseGame = false;
    sf::RenderWindow window(sf::VideoMode(590,500), "Pacman");
    pthread_mutex_init(&PlayerMutex ,NULL);
    sem_init(&GhostKeys,0,2);
    sem_init(&GhostPermits,0,2);
    sem_init(&SpeedBoost,0,2);
    pthread_mutex_init(&ReadWriteMutex ,NULL);  
    
    //score
    Text Score;
    sf::Font font;//new font declared
    if (!font.loadFromFile("Bungee-Regular.ttf"))//incase font does not load
    {
        std::cout<<"NOFONT"<<std::endl;
    }
    Score.setFont(font); //score displayed
    Score.setCharacterSize(20);
    Score.setPosition(10,450);
    Score.setString("SCORE: ");

    //life
    Sprite Lifes[3];
    Texture Lifetex;
    Lifetex.loadFromFile("pacman-art/pacman-right/2.png");
    Lifes[0].setTexture(Lifetex);
    Lifes[1].setTexture(Lifetex);
    Lifes[2].setTexture(Lifetex);

    //maze
    sf::Color darkBlue(31, 81, 255);
    for(int i = 0; i < 250; i++)
    {
        wall[i].setSize(sf::Vector2f(20,20));
        wall[i].setFillColor(darkBlue);
    }
    SetMaze(window,maze,wall);

    //pellet
   int pelletCount=0;
   int fruitCount = 0;
   SetPellets(Normal_Pellets,pelletCount, fruitCount);

    //enemy
    pthread_t ghostThread[4];
    int index_Ghost = 1;
	int index2 = 2;
	int index3 = 3;
	int index4 = 4;
    pthread_mutex_init(&GhostMutex[0], NULL);
    pthread_create(&ghostThread[0], NULL, Ghost_Movement, (void*) &index_Ghost);
    pthread_mutex_init(&GhostMutex[1], NULL);
    pthread_create(&ghostThread[1], NULL, Ghost_Movement, (void*) &index2);
    pthread_mutex_init(&GhostMutex[2], NULL);
    pthread_create(&ghostThread[2], NULL, Ghost_Movement, (void*) &index3);
    pthread_mutex_init(&GhostMutex[3], NULL);
    pthread_create(&ghostThread[3], NULL, Ghost_Movement, (void*) &index4);

    pthread_t Player_ID = 0;
    pthread_create(&Player_ID,NULL,updateFunction,NULL);
    sf::Clock clock;
    while (window.isOpen())
    {
        timing = clock.getElapsedTime().asSeconds();
        clock.restart();
        srand(time(0));
        timer += timing;
        //cout<<"timer: "<<timer<<endl;

        sf::Event event;

        while (window.pollEvent(event))
        {  
            if (event.type == Event::Closed || P->Lives <= 0)
            {
                CloseGame = true;
                GameOpen = false;
                window.close();
            }
            if(event.type == Event::KeyPressed)
            {
                P->GotUserKeyPress(event.key.code);
            }

        }

        UnPowerMode();
        reSpawnPellets(timer,fruitCount);
        Score.setString("SCORE  " + std::to_string(P->score));

        window.clear();

       
		
        pthread_mutex_lock(&PlayerMutex);
        window.draw(P->sprite);
        pthread_mutex_unlock(&PlayerMutex);
      

        DisplayMaze(window,maze,wall);
        DisplayPellets(window,Normal_Pellets,pelletCount);
        window.draw(Score);
        DrawLife(window,Lifes);
       	for(int i=0; i<4; i++)
		{
			pthread_mutex_lock(&GhostMutex[i]);
			Ghosts[i].draw(window);
			pthread_mutex_unlock(&GhostMutex[i]);
		}
		
        window.display();
    }

    pthread_mutex_destroy(&PlayerMutex);

    return 0;
}
