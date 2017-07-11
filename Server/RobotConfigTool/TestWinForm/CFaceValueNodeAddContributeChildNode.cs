using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
namespace TestWinForm
{
    class CFaceValueNodeAddContributeChildNode : addChildPanel
    {
        protected RadioButton[] vContributeBtn = new RadioButton[3];
        public override void init(Form11 pForm)
        {
            base.init(pForm);
            m_myPanel = pForm.getAddContributePanel();
            int nidx = 0;
            foreach (RadioButton btn in pForm.m_vContributeBtns)
            {
                vContributeBtn[nidx++] = btn;
            }
        }

        public override void refreshCurNodeData(NodeData nCurData)
        {
            base.refreshCurNodeData(nCurData);
            bool isAllAdd = true;
            for (int nIdx = 0; nIdx < 3; ++nIdx)
            {
                if (vContributeBtn[nIdx] == null)
                {
                    continue;
                }

                if ( nCurData.m_pParentNode.m_pParentNode.m_nValue == 0)
                {
                    if (nIdx != 2)
                    {
                        vContributeBtn[nIdx].Enabled =  false;
                        vContributeBtn[nIdx].Checked = false;
                        continue;
                    }
                }

                vContributeBtn[nIdx].Enabled = nCurData.isNodeValueAlreadyExist(nIdx) == false;
                vContributeBtn[nIdx].Checked = false;
                if ( vContributeBtn[nIdx].Enabled )
                    isAllAdd = false;
            }
            m_panel.enableAddNodeBtn = isAllAdd == false;
            m_descLabel.Text = string.Format("当前是{0},\n牌型是{1} \n请选择配置手牌贡献了几张牌", nCurData.m_pParentNode.m_pParentNode.desc, nCurData.desc);
        }

        public override TreeNode doAddChildNode()
        {
            int nIdx = 0;
            int nRealIdx = -1;
            for (; nIdx < 3; ++nIdx)
            {
                if (vContributeBtn[nIdx] == null || vContributeBtn[nIdx].Enabled == false)
                {
                    continue;
                }

                if (vContributeBtn[nIdx].Checked)
                {
                    nRealIdx = nIdx;
                    break;
                }
            }

            if (nRealIdx == -1)
            {
                MessageBox.Show("请选择要配置贡献几张牌");
                return null;
            }

            NodeContribute pNodeData = new NodeContribute(nRealIdx);
            if (m_pCurNodeData.addChildNode(pNodeData) == null)
            {
                MessageBox.Show("add node error , already exist");
                vContributeBtn[nIdx].Enabled = false;
                vContributeBtn[nIdx].Checked = false;
                return null;
            }

            TreeNode pnode = new TreeNode();
            pnode.Tag = pNodeData;
            pnode.Text = pNodeData.desc;

            refreshCurNodeData(m_pCurNodeData);
            return pnode;
        }
    }
}
