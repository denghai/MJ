using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
namespace TestWinForm
{
    class CRoundAddCardTypeChildNode
        : addChildPanel
    {
        protected RadioButton[] vCardTypeBtn;
        public override void init(Form11 pForm)
        {
            base.init(pForm);
            m_myPanel = pForm.getAddCardTypePanel();
            vCardTypeBtn = new RadioButton[(int)eCardType.eCard_Robot_Max];
            vCardTypeBtn = pForm.m_vCardType;
            vCardTypeBtn[0] = null; 
        }

        public override void refreshCurNodeData(NodeData nCurData)
        {
            base.refreshCurNodeData(nCurData);
            bool isAllAdd = true;
            for (int nIdx = 0; nIdx < (int)eCardType.eCard_Robot_Max; ++nIdx)
            {
                if (vCardTypeBtn[nIdx] == null)
                {
                    continue;
                }

                if ( nCurData.m_nValue == 0)
                {
                    if (nIdx != (int)eCardType.eCard_YiDui && nIdx != (int)eCardType.eCard_TongHua && nIdx != (int)eCardType.eCard_ShunZi && nIdx != (int)eCardType.eCard_GaoPai )
                    {
                        vCardTypeBtn[nIdx].Enabled = false;
                        vCardTypeBtn[nIdx].Checked = false;
                        continue;
                    }
                }

                vCardTypeBtn[nIdx].Enabled = nCurData.isNodeValueAlreadyExist(nIdx) == false;
                vCardTypeBtn[nIdx].Checked = false;
                if (vCardTypeBtn[nIdx].Enabled)
                    isAllAdd = false;
            }
            m_panel.enableAddNodeBtn = isAllAdd == false;
            m_descLabel.Text = string.Format("当前是{0}，请选择配置的牌型", nCurData.desc);
        }

        public override TreeNode doAddChildNode()
        {
            int nIdx = 0;
            int nFindIdx = -1;
            for (; nIdx < (int)eCardType.eCard_Robot_Max; ++nIdx)
            {
                if (vCardTypeBtn[nIdx] == null || vCardTypeBtn[nIdx].Enabled == false)
                {
                    continue;
                }

                if (vCardTypeBtn[nIdx].Checked)
                {
                    nFindIdx = nIdx;
                    break;
                }
            }

            if (nFindIdx == -1)
            {
                MessageBox.Show("请选择一个要配置的牌型");
                return null;
            }

            NodeCardType pNodeData = new NodeCardType(nIdx);
            if (m_pCurNodeData.addChildNode(pNodeData) == null)
            {
                MessageBox.Show("add node error , already exist");
                vCardTypeBtn[nIdx].Enabled = false;
                vCardTypeBtn[nIdx].Checked = false;
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
