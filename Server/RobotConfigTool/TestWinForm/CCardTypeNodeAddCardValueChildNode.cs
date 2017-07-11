using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
namespace TestWinForm
{
    class CCardTypeNodeAddCardValueChildNode : addChildPanel
    {
        protected RadioButton[] vCardFaceValue = new RadioButton[15];
        public override void init(Form11 pForm)
        {
            base.init(pForm);
            m_myPanel = pForm.getAddCardValuePanel();
            //int nidx = 0 ;
            vCardFaceValue = pForm.m_vCardFaceValue;
            //foreach ( RadioButton btn in pForm.m_vCardFaceValue )
            //{
            //    vCardFaceValue[nidx++] = btn;
            //}
        }

        public override void refreshCurNodeData(NodeData nCurData)
        {
            base.refreshCurNodeData(nCurData);
            bool isAllAdd = true;
            for (int nIdx = 0; nIdx < 15; ++nIdx)
            {
                if (vCardFaceValue[nIdx] == null)
                {
                    continue;
                }

                if (nIdx == 0)
                {
                    if ( m_pCurNodeData.m_pParentNode.m_nValue != 3 )
                    {
                        vCardFaceValue[nIdx].Enabled =  false;
                        vCardFaceValue[nIdx].Checked = false;
                        continue;
                    }
                }

                vCardFaceValue[nIdx].Enabled = nCurData.isNodeValueAlreadyExist(nIdx) == false;
                vCardFaceValue[nIdx].Checked = false;
                if (vCardFaceValue[nIdx].Enabled)
                    isAllAdd = false;
            }
            m_panel.enableAddNodeBtn = isAllAdd == false;
            m_descLabel.Text = string.Format("当前是{0},\n牌型是{1} \n请选择配置牌面值", nCurData.m_pParentNode.desc, nCurData.desc);
        }

        public override TreeNode doAddChildNode()
        {
            int nIdx = 0, nFindIdx = -1 ;
            for (; nIdx < 15; ++nIdx)
            {
                if (vCardFaceValue[nIdx] == null || vCardFaceValue[nIdx].Enabled == false)
                {
                    continue;
                }

                if (vCardFaceValue[nIdx].Checked)
                {
                    nFindIdx = nIdx;
                    break;
                }
            }

            if (nFindIdx == -1)
            {
                MessageBox.Show("请选择一个配置的牌面值");
                return null;
            }

            NodeCardValue pNodeData = new NodeCardValue(nFindIdx);
            if (m_pCurNodeData.addChildNode(pNodeData) == null)
            {
                MessageBox.Show("add node error , already exist");
                vCardFaceValue[nFindIdx].Enabled = false;
                vCardFaceValue[nFindIdx].Checked = false;
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
