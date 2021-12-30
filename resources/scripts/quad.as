string linkedObj;

void main(string linkedObjectName)
{
	Print("Hello from Quad! -> %s", linkedObjectName);
	linkedObj = linkedObjectName;
	yield();
	createCoroutine(routine);
}


void routine()
{
	yield();
	Print("Quad Coroutine! -> %s", linkedObj);
}