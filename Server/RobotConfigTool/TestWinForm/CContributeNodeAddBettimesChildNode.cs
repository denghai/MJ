using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
namespace TestWinForm
{
    class CContributeNodeAddBettimesChildNode : addChildPanel
    {
        protected NumericUpDown m_pBetTimes, m_pPass, m_pFollow, m_pAllIn, m_pAdd, m_pAddLowLimit, m_pAddTopLimit;
        public override void init(Form11 pForm)
        {
            base.init(pForm);
            m_myPanel = pForm.getAddBetTimesPanel();
            m_pBetTimes = pForm.getBetTimes();
            m_pPass = pForm.getPass();
            m_pFollow = pForm.getFollow();
            m_pAllIn = pForm.getAllIn();
            m_pAdd = pForm.getAdd();
            m_pAddLowLimit = pForm.getAddLowLimit();
            m_pAddTopLimit = pForm.getAddTopLimit();
        }

        public override void refreshCurNodeData(NodeData nCurData)
        {
            base.refreshCurNodeData(nCurData);
            m_panel.enableAddNodeBtn = true;
            m_descLabel.Text = string.Format("当前是{0},\n最终牌型是{1},\n{2}，\n请配置操作策略", nCurData.m_pParentNode.m_pParentNode.m_pParentNode.desc, nCurData.m_pParentNode.desc, nCurData.desc);
        }

        public override TreeNode doAddChildNode()
        {
            int nIdx = (int)m_pBetTimes.Value;
            if (m_pCurNodeData.isNodeValueAlreadyExist(nIdx))
            {
                MessageBox.Show("already have this bet times node");
                return null;
            }


            if (m_pFollow.Value + m_pPass.Value + m_pAdd.Value + m_pAllIn.Value != 100)
            {
                MessageBox.Show("权重总和不等于100");
                return null;
            }

            if ( m_pAdd.Value > 0 && m_pAddLowLimit.Value > m_pAddTopLimit.Value)
            {
                MessageBox.Show("加注权重下限大于上限");
                return null;
            }

            NodeBettimes pNodeData = new NodeBettimes(nIdx);
            pNodeData.nFollowRate = (int)m_pFollow.Value;
            pNodeData.nPassRate = (int)m_pPass.Value;
            pNodeData.nAddRate = (int)m_pAdd.Value;
            pNodeData.nALLInRate = (int)m_pAllIn.Value;
            pNodeData.nAddLowLimit = (int)m_pAddLowLimit.Value;
            pNodeData.nAddTopLimit = (int)m_pAddTopLimit.Value;

            if (pNodeData.checkAllValueValid() == false )
            {
                MessageBox.Show("add node error , value not valid, rate toall must = 100 , uplimit should bigger than lowLimit");
                return null;
            }

            if (m_pCurNodeData.addChildNode(pNodeData) == null)
            {
                MessageBox.Show("add node error , value not valid, rate toall must = 100 , uplimit should bigger than lowLimit");
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
