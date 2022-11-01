package taxi;

public class State implements Comparable<State>
{
	public long steps;
	public byte length;
	public State parentH;
	public State parentV;
	public State horizontal;
	public State vertical;
	public int index;
//	public int parentCount;
	
	public State(long steps, byte length)
	{
		this.steps = steps;
		this.length = length;
		this.index = -1;
//		parentCount = 0;
	}
	
	@Override
	public int compareTo(State o)
	{
		if(length != o.length)
		{
			return length - o.length;
		}
		// Stolen from Java Long.
		return (steps < o.steps) ? -1 : ((steps == o.steps) ? 0 : 1);
	}
	
	public boolean equals(State o)
	{
		return o != null && length == o.length && steps == o.steps;
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
