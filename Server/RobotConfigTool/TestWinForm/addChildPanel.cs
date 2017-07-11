using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;
namespace TestWinForm
{
    abstract class addChildPanel
    {
        protected Form11 m_panel;
        protected NodeData m_pCurNodeData;
        protected Label m_descLabel;
        protected Panel m_myPanel = null;
        public virtual void init(Form11 pannel)
        {
            m_pCurNodeData = null;
            m_panel = pannel;
            m_descLabel = pannel.getDescLabel();
        }

        public void show()
        {
            if (m_myPanel != null)
            {
                m_myPanel.Show();
                m_myPanel.Location = new Point(430,120);
            }
        }

        public void hide()
        {
            if (m_myPanel != null)
                m_myPanel.Hide();
        }

        public virtual void refreshCurNodeData(NodeData nCurData)
        {
            m_pCurNodeData = nCurData;
        }

        public abstract TreeNode doAddChildNode();
    }

}
