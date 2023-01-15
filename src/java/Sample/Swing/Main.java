import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.List;
import java.util.concurrent.ExecutionException;
import javax.swing.*;


// From: https://stackoverflow.com/questions/782265/how-do-i-use-swingworker-in-java
class Foo {
void makeGUI()
{
    final JFrame f = new JFrame();
    f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    f.getContentPane().setLayout(new FlowLayout());

    // include: "class AnswerWorker" code here.
class AnswerWorker extends SwingWorker<Integer, Integer>
{
    protected Integer doInBackground() throws Exception
    {
        // Do a time-consuming task.
        Thread.sleep(1000);
        return 42;
    }

    protected void done()
    {
        try
        {
            JOptionPane.showMessageDialog(f, get());
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }
} // class AnswerWorker

    // include: "JButton" b code here.
JButton b = new JButton("Answer!");
b.addActionListener(new ActionListener() {
    public void actionPerformed(ActionEvent e)
    {
        new AnswerWorker().execute();
    }
});

    f.getContentPane().add(b);
    f.getContentPane().add(new JButton("Nothing"));
    f.pack();
    f.setVisible(true);
} // makeGUI
} // class Foo

//----------------------------------------------------------------------------
// Added, as was class Foo wrapper
public class Main {
    public static void main(String[] args) {
        for(int i= 0; i<args.length; ++i)
            System.out.println(args[i]);

        Foo f= new Foo();
        f.makeGUI();
    }
}
