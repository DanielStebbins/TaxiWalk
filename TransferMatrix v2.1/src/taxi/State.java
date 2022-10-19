package taxi;

public class State
{
	public long steps;
	public byte length;
	public State horizontal;
	public State vertical;
	public int index;
	
	public State(long steps, byte length)
	{
		this.steps = steps;
		this.length = length;
	}
	
	@Override
	public String toString()
	{
		// Prints left to right, opposite of how it's stored.
		String binaryString = "";
		for(int i = 0; i < length; i++)
		{
			if(((steps >>> i) & 1) == 1)
			{
				binaryString += "V";
			}
			else
			{
				binaryString += "H";
			}
		}
		return "Length: " + length + ", " + binaryString;
	}
}
