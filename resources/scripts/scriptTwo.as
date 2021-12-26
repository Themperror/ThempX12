void main()
{
	print("Hello from the two side!");
	
	print("creating scriptTwoCoroutineA");
	createCoRoutine(scriptTwoCoroutineA);
	print("creating scriptTwoCoroutineB");
	createCoRoutine(scriptTwoCoroutineB);
	int i = 0;
	while(true)
	{
		i++;
		yield();
		yield();
		print("Looping!! %f", i * 0.333f); 
		yield();
		yield(); 
		if(i > 10)
		{
			break;
		}
	}
}

void scriptTwoCoroutineA()
{
	print("created scriptTwoCoroutineA");
	for(int i = 0; i < 20; i++)
	{
		print("scriptTwoCoroutineA call One %i", i);
		yield();
		yield();
		yield();
		print("scriptTwoCoroutineA call Two %i", i);
		yield();
		yield();
		print("scriptTwoCoroutineA call Three %i", i);
	}
}

void scriptTwoCoroutineB()
{
	print("created scriptTwoCoroutineB");
	print("scriptTwoCoroutineB call %s", "One");
	yield();
	print("scriptTwoCoroutineB call %s", "Two");
}