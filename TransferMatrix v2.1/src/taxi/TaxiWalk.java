// Record: N=47 in 7.8 seconds.

package taxi;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.TreeMap;

public class TaxiWalk
{
	// The length of walk to enumerate. MAX 64 CURRENTLY (long encoding).
	public static final int N = 47;
	
	// These constants relate to the previously calculated steps to the origin file.
	public static final int MAX_N = 100;
	public static final int DIM = 2 * MAX_N + 1;
	public static final int OFFSET = DIM * DIM;
	public static final int LATTICE_SIZE = OFFSET * 4;
	public static int[] stepsToOrigin = new int[LATTICE_SIZE];
	
	// Used for building the automaton.
	public static State genesis = new State(0L, (byte) 0);
	public static State twoNullPointers = new State(0L, (byte) 0);
	public static int count = 1;
	public static LinkedList<State> untreated = new LinkedList<State>();
	
	public static long findEndpoint = 0;
	public static long hasLoop = 0;
	public static long reduce = 0;
	public static long contains = 0;
	public static long treePruning = 0;
	
	public static void main(String args[])
	{
		// Read the steps to the origin from every point, which have been previously calculated and stored.
		try
		{
			BufferedReader reader = new BufferedReader(new FileReader(new File("StepsToOrigin.txt")));
			String[] split = reader.readLine().split(" ");
			for(int i = 0; i < LATTICE_SIZE; i++)
			{
				stepsToOrigin[i] = Integer.parseInt(split[i]);
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		
		for(int n = 15; n <= N; n += 4)
		{
			long startTime = System.currentTimeMillis();
			
			// Set up the automaton stuff.
			untreated.addLast(genesis);
			
			// Main automaton-generating code.
			long steps = 0L;
			byte length = 0;
			State end = new State(steps, length);
			while(!untreated.isEmpty())
			{
				State start = untreated.removeFirst();
				
				
				
	//			System.out.println("\n" + start);
				
				// Try to take a horizontal step.
				if(start.length < 2 || approach(start.steps, start.length) != 1)
				{
					steps = start.steps;
					length = (byte) (start.length + 1);
					
	//				System.out.println("Horizontal: " + pattern);
					
					// Find end point.
					long time = System.currentTimeMillis();
					int endX = 0;
					int xStep = 1;
					int endY = 0;
					int yStep = 1;
					for(int i = 0; i < length; i++)
					{
						if((steps >>> i & 1) == 0)
						{	
							endX += xStep;
							yStep = -yStep;
						}
						else
						{
							endY += yStep;
							xStep = -xStep;
						}
					}
					findEndpoint += System.currentTimeMillis() - time;
					
					
					// Check for loop.		
					if(!hasLoop(steps, length, endX, endY))
					{
						// If cannot loop, chop off first step.
						time = System.currentTimeMillis();
						while(stepsToOrigin[approach(steps, length) * OFFSET + (endX + MAX_N) * DIM + endY + MAX_N] > n - length)
						{
							if((steps & 1) == 0)
							{
								endX -= 1;
								endY = -endY;
							}
							else
							{
								endX = -endX;
								endY -= 1;
							}
							steps = steps >>> 1;
							length--;
							
							if((steps & 1) == 1)
							{
								steps = steps ^ ((1L << length) - 1);
								int temp = endX;
								endX = endY;
								endY = temp;
							}
						}
						
						// If first step is now vertical, flip to horizontal. "ST" encoding faster?
						if((steps & 1) == 1)
						{
							steps = steps ^ ((1L << length) - 1);
						}
						reduce += System.currentTimeMillis() - time;
						
						
						time = System.currentTimeMillis();
						end.steps = steps;
						end.length = length;
						State temp = putIfAbsent(end);
						if(temp == null)
						{
	//						System.out.println("Added");
							// Added to tree.
							start.horizontal = end;
							untreated.addLast(end);
							end = new State(steps, length);
						}
						else
						{
	//						System.out.println("Present");
							// Present in tree.
							start.horizontal = temp;
						}
						contains += System.currentTimeMillis() - time;
					}
				}
				
				
				// Try to take a vertical step.
				if(start.length < 2 || approach(start.steps, start.length) != 2)
				{
					steps = start.steps | (1L << start.length);
					length = (byte) (start.length + 1);
					
	//				System.out.println("Vertical: " + pattern);
				
					// Find end point.
					long time = System.currentTimeMillis();
					int endX = 0;
					int xStep = 1;
					int endY = 0;
					int yStep = 1;
					for(int i = 0; i < length; i++)
					{
						if((steps >>> i & 1) == 0)
						{	
							endX += xStep;
							yStep = -yStep;
						}
						else
						{
							endY += yStep;
							xStep = -xStep;
						}
					}
					findEndpoint += System.currentTimeMillis() - time;
					
					
					// Check for loop.		
					if(!hasLoop(steps, length, endX, endY))
					{
						// If cannot loop, chop off first step.
						time = System.currentTimeMillis();
						while(stepsToOrigin[approach(steps, length) * OFFSET + (endX + MAX_N) * DIM + endY + MAX_N] > n - length)
						{
							if((steps & 1) == 0)
							{
								endX -= 1;
								endY = -endY;
							}
							else
							{
								endX = -endX;
								endY -= 1;
							}
							steps = steps >>> 1;
							length--;
							
							if((steps & 1) == 1)
							{
								steps = steps ^ ((1L << length) - 1);
								int temp = endX;
								endX = endY;
								endY = temp;
							}
						}
						
						// If first step is now vertical, flip to horizontal.
						if((steps & 1) == 1)
						{
							steps = steps ^ ((1L << length) - 1);
						}
	//					System.out.println("Reduced to: " + pattern);
						reduce += System.currentTimeMillis() - time;
						
						
						time = System.currentTimeMillis();
						end.steps = steps;
						end.length = length;
						State temp = putIfAbsent(end);
						if(temp == null)
						{
							// Added to tree.
	//						System.out.println("Added");
							start.vertical = end;
							untreated.addLast(end);
							end = new State(steps, length);
						}
						else
						{
							// Present in tree.		
	//						System.out.println("Present");
							start.vertical = temp;
						}
						contains += System.currentTimeMillis() - time;
					}
				}
				long time = System.currentTimeMillis();
				if(start.horizontal == null && start.vertical == null)
				{
					start = twoNullPointers;
					count--;
				}
				treePruning += System.currentTimeMillis() - time;
			}
			long endTime = System.currentTimeMillis();
			System.out.println("\nN: " + n);
			System.out.println("Automaton Size: " + count);
			System.out.println("Total Time: " + (endTime - startTime) / 1000.0 + "\n");
			
			System.out.println("Find Endpoint: " + findEndpoint / 1000.0 + "(" + Math.round((double) findEndpoint / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Has Loop: " + hasLoop / 1000.0 + "(" + Math.round((double) hasLoop / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Reduce Pattern: " + reduce / 1000.0 + "(" + Math.round((double) reduce / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Tree Contains: " + contains / 1000.0 + "(" + Math.round((double) contains / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Tree Pruning: " + treePruning / 1000.0 + "(" + Math.round((double) treePruning / (endTime - startTime) * 1000) / 10.0 + "%)");
			
			// Running the automaton.
			TreeMap<State, Integer> current = new TreeMap<State, Integer>();
			TreeMap<State, Integer> next = new TreeMap<State, Integer>();
			current.put(genesis, 1);
			for(int i = 1; i <= n; i++)
			{
				for(State start : current.keySet())
				{
					if(start.horizontal != null)
					{
						next.put(start.horizontal, next.getOrDefault(start.horizontal, 0) + current.get(start));
					}
					
					if(start.vertical != null)
					{
						next.put(start.vertical, next.getOrDefault(start.vertical, 0) + current.get(start));
					}
				}
				current = next;
				next = new TreeMap<State, Integer>();
			}
	
			long taxi = 0;
			for(State s : current.keySet())
			{
				taxi += current.get(s);
			}
			System.out.println("\n" + taxi);
			
			genesis = new State(0L, (byte) 0);
			count = 0;
			untreated.clear();
			
			findEndpoint = 0;
			hasLoop = 0;
			reduce = 0;
			contains = 0;
			treePruning = 0;
		}
	}
	
	public static void unlink(State s)
	{
		if(s.horizontal != null)
		{
			unlink(s.horizontal);
			s.horizontal = null;
		}
		if(s.vertical != null)
		{
			unlink(s.vertical);
			s.vertical = null;
		}
	}
	
	public static int approach(long steps, byte length)
	{
		return (int) ((steps >>> (length - 2) & 1) * 2 + (steps >>> (length - 1)));
	}
	
	public static boolean hasLoop(long steps, byte length, int endX, int endY)
	{
		long time = System.currentTimeMillis();
		int x = 0;
		int xStep = 1;
		int y = 0;
		int yStep = 1;
		boolean loop = x == endX && y == endY;
		int i = 0;
		while(!loop && i < length - 12)
		{					
			if((steps >>> i & 1) == 0)
			{	
				x += xStep;
				yStep = -yStep;
			}
			else
			{
				y += yStep;
				xStep = -xStep;
			}
			loop = x == endX && y == endY;
			i++;
		}
		hasLoop += System.currentTimeMillis() - time;
		return loop;
	}
	
	public static State putIfAbsent(State s)
	{
		State parent = genesis;
		for(int i = 0; i < s.length - 1; i++)
		{
			if(((s.steps >>> i) & 1) == 1)
			{
				parent = parent.vertical;
			}
			else
			{
				parent = parent.horizontal;
			}
		}
		
		if(((s.steps >>> (s.length - 1)) & 1) == 1)
		{
			if(parent.vertical == null)
			{
//				System.out.println("Adding");
				count++;
				parent.vertical = s;
				return null;
			}
			else
			{
//				System.out.println("Present");
				return parent.vertical;
			}
		}
		else
		{
			if(parent.horizontal == null)
			{
//				System.out.println("Adding");
				count++;
				parent.horizontal = s;
				return null;
			}
			else
			{
//				System.out.println("Present");
				return parent.horizontal;
			}
		}
	}
}
