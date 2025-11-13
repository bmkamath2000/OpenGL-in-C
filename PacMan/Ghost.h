#ifndef GHOST_H
#define GHOST_H
class Ghost
{
   private:

  public:
         bool edible;
	     int edible_max_time;
	     int edible_timer;
         bool eaten;
	     bool transporting;
         float color[3];
	     double speed;
	     double max_speed;
	     bool in_jail;
	     int jail_timer;
	     double angle;
 	     double x, y;

	     Ghost(double, double);


                ~Ghost(void);

	     void Move(); //Move the Monster

	     void Update(void);  //Update Monster State

	     void Chase(double, double, bool*);  //Chase Pacman

	     bool Catch(double, double);	//collision detection

	     void Reinit(void);

	     void Vulnerable(void);

	     void Draw(void);   //Draw the Monster
	     void game_over(void);

};
#endif
