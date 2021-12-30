string linkedObj;

void main(string linkedObjectName)
{
	print("Hello from Quad! -> %s", linkedObjectName);
	linkedObj = linkedObjectName;
	yield();
	createCoroutine(routine);
}


void routine()
{
	yield();
	print("Quad Coroutine! -> %s", linkedObj);
}