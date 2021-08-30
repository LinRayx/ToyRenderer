#include "RenderSystem/RenderLoop.h"

using namespace std;

int WIDTH = 800;
int HEIGHT = 600;




int main()
{
	RenderSystem::RenderLoop loop;
	loop.Run();
	cout << "hello" << endl;
	return 0;
}