package taxi;

public class Automaton
{
	public State root;
	
	public int size;
	
	public Automaton(State root)
	{
		this.root = root;
		this.size = 1;
	}
	
	public State putIfAbsent(State s)
	{
//		System.out.println("Searching for: " + s.toString());
		State prev = null;
		State current = root;
		boolean left = true;
        while(current != null)
        {
            int cmp = s.pattern.compareTo(current.pattern);
            if(cmp < 0)
            {
            	prev = current;
                current = current.left;
                left = true;
            }
            else if(cmp > 0)
            {
            	prev = current;
                current = current.right;
                left = false;
            }
            else
            {
            	// Found.
                return current;
            }
        }
        // Not found.
        if(left)
        {
        	prev.left = s;
        }
        else
        {
        	prev.right = s;
        }
    	s.parent = prev;
        fixAfterInsertion(s);
        size++;
//        System.out.println("Not Found");
        return null;
	}
	
	// Stolen from TreeMap.
	private void fixAfterInsertion(State s)
	{
        s.color = true;
        while(s != null && s != root && s.parent.color == true)
        {
            if(parentOf(s) == leftOf(parentOf(parentOf(s))))
            {
                State y = rightOf(parentOf(parentOf(s)));
                if(colorOf(y) == true)
                {
                    setColor(parentOf(s), false);
                    setColor(y, false);
                    setColor(parentOf(parentOf(s)), true);
                    s = parentOf(parentOf(s));
                }
                else
                {
                    if(s == rightOf(parentOf(s)))
                    {
                        s = parentOf(s);
                        rotateLeft(s);
                    }
                    setColor(parentOf(s), false);
                    setColor(parentOf(parentOf(s)), true);
                    rotateRight(parentOf(parentOf(s)));
                }
            }
            else
            {
                State y = leftOf(parentOf(parentOf(s)));
                if(colorOf(y) == true)
                {
                    setColor(parentOf(s), false);
                    setColor(y, false);
                    setColor(parentOf(parentOf(s)), true);
                    s = parentOf(parentOf(s));
                }
                else
                {
                    if(s == leftOf(parentOf(s)))
                    {
                        s = parentOf(s);
                        rotateRight(s);
                    }
                    setColor(parentOf(s), false);
                    setColor(parentOf(parentOf(s)), true);
                    rotateLeft(parentOf(parentOf(s)));
                }
            }
        }
        root.color = false;
    }
	
    private boolean colorOf(State s)
    {
        return (s == null ? false : s.color);
    }

    private State parentOf(State s)
    {
        return (s == null ? null: s.parent);
    }

    private void setColor(State s, boolean c)
    {
        if (s != null)
            s.color = c;
    }

    private State leftOf(State s)
    {
        return (s == null) ? null: s.left;
    }

    private State rightOf(State s)
    {
        return (s == null) ? null: s.right;
    }

    private void rotateLeft(State s)
    {
        if (s != null)
        {
            State r = s.right;
            s.right = r.left;
            if (r.left != null)
                r.left.parent = s;
            r.parent = s.parent;
            if(s.parent == null)
                root = r;
            else if(s.parent.left == s)
                s.parent.left = r;
            else
                s.parent.right = r;
            r.left = s;
            s.parent = r;
        }
    }

    private void rotateRight(State s)
    {
        if (s != null)
        {
            State l = s.left;
            s.left = l.right;
            if (l.right != null) l.right.parent = s;
            l.parent = s.parent;
            if (s.parent == null)
                root = l;
            else if (s.parent.right == s)
                s.parent.right = l;
            else s.parent.left = l;
            l.right = s;
            s.parent = l;
        }
    }
}
