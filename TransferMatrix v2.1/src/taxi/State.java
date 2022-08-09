package taxi;

public class State implements Comparable<State>
{
	public String pattern;
	public State horizontal;
	public State vertical;
	
	public State(String pattern)
	{
		this.pattern = pattern;
	}

	@Override
	public int compareTo(State o)
	{
		return pattern.compareTo(o.pattern);
	}
	
	@Override
	public String toString()
	{
		return "\"" + pattern + "\"";
	}
}
