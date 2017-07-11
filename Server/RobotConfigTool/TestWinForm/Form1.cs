using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.IO;
namespace TestWinForm
{
    public enum eDocState
    {
        eDoc_None,
        eDoc_NewCreated,
        eDoc_NewOpen,
        eDoc_Modified,
        eDoc_AlreadySaved,
        eDoc_Max,
    }

    public partial class Form11 : Form
    {  
        public Label getDescLabel() { return this.desclabel; }

        // card type
        public RadioButton[] m_vCardType = new RadioButton[(int)eCardType.eCard_Robot_Max];
        // card value .
        public RadioButton[] m_vCardFaceValue = new RadioButton[15] ;
        // contribute btns ;
        public RadioButton[] m_vContributeBtns = new RadioButton[3];

        // bet times 
        public NumericUpDown getBetTimes() { return this.bettimes; }
        public NumericUpDown getFollow() { return this.follow; }
        public NumericUpDown getPass() { return this.pass; }
        public NumericUpDown getAllIn() { return this.all; }
        public NumericUpDown getAdd() { return this.add; }
        public NumericUpDown getAddLowLimit() { return this.addlowlimit; }
        public NumericUpDown getAddTopLimit() { return this.addtoplimit; }

        public Panel getAddCardTypePanel() { return this.addcardtype; }
        public Panel getAddCardValuePanel() { return this.addcardfacevlue; }
        public Panel getAddContributePanel() { return this.addcontribute; }
        public Panel getAddBetTimesPanel() { return this.addbettimes; }

        protected eNodeType m_nCurNodeType = eNodeType.eNode_Max;
        protected eDocState m_eDocState = eDocState.eDoc_None;
        public static string s_file_ext = ".ai";
        public static string s_unfinish_flag = "_unfinish";
        public bool enableAddNodeBtn
        {
            get { return this.addNodeBtn.Enabled; }
            set { this.addNodeBtn.Enabled = value; }
        }

        addChildPanel[] m_vAddSettingPanel = new addChildPanel[(int)eNodeType.eNode_Max];

        public string m_strCurDocPath = null;
        public Form11()
        {
            InitializeComponent();

            // card type 
            m_vCardType[(int)eCardType.eCard_TongHuaShun] = this.tonghuashun; ;
            m_vCardType[(int)eCardType.eCard_SiTiao] = this.sitiao;
            m_vCardType[(int)eCardType.eCard_HuLu] = this.hulu;
            m_vCardType[(int)eCardType.eCard_TongHua] = this.tonghua;
            m_vCardType[(int)eCardType.eCard_ShunZi] = this.shunzi;
            m_vCardType[(int)eCardType.eCard_SanTiao] = this.santiao;
            m_vCardType[(int)eCardType.eCard_LiangDui] = this.liangdui;
            m_vCardType[(int)eCardType.eCard_YiDui] = this.yidui;
            m_vCardType[(int)eCardType.eCard_GaoPai] = this.gaopai;
            m_vCardType[(int)eCardType.eCard_Robot_GaoPai4TongHua] = this.gaopai4tonghua;
            m_vCardType[(int)eCardType.eCard_Robot_GaoPai4ShunZi] = this.gaopai4shunzi;

            // card face value 
            m_vCardFaceValue[1] = null;
            m_vCardFaceValue[0] = this.v0;
            m_vCardFaceValue[2] = this.v2;
            m_vCardFaceValue[3] = this.v3;
            m_vCardFaceValue[4] = this.v4;
            m_vCardFaceValue[5] = this.v5;
            m_vCardFaceValue[6] = this.v6;
            m_vCardFaceValue[7] = this.v7;
            m_vCardFaceValue[8] = this.v8;
            m_vCardFaceValue[9] = this.v9;
            m_vCardFaceValue[10] = this.v10;
            m_vCardFaceValue[11] = this.v11;
            m_vCardFaceValue[12] = this.v12;
            m_vCardFaceValue[13] = this.v13;
            m_vCardFaceValue[14] = this.v14;

            // conribute btns 
            m_vContributeBtns[0] = this.con0;
            m_vContributeBtns[1] = this.con1;
            m_vContributeBtns[2] = this.con2;

            setupAddChildPannel();
            enableAddNodeBtn = false;
            this.btnRemoveNode.Enabled = false;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        protected void hideAllNodeSettingPanel()
        {
            getAddCardTypePanel().Hide();
            getAddCardValuePanel().Hide();
            getAddContributePanel().Hide();
            getAddBetTimesPanel().Hide();
        }

        private void setupAddChildPannel()
        {
            m_vAddSettingPanel[(int)eNodeType.eNode_Round] = new CRoundAddCardTypeChildNode();
            m_vAddSettingPanel[(int)eNodeType.eNode_CardType] = new CCardTypeNodeAddCardValueChildNode();
            m_vAddSettingPanel[(int)eNodeType.eNode_CardValue] = new CFaceValueNodeAddContributeChildNode();
            m_vAddSettingPanel[(int)eNodeType.eNode_ContributeCard] = new CContributeNodeAddBettimesChildNode();
            m_vAddSettingPanel[(int)eNodeType.eNode_BetTimes] = new BetTimesNodeDisplay();

            foreach ( addChildPanel argpanel in m_vAddSettingPanel )
            {
                argpanel.init(this);
                argpanel.hide();
            }
          
        }

        private void addNodeBtnClick(object sender, EventArgs e)
        {
            TreeNode addNode = m_vAddSettingPanel[(int)m_nCurNodeType].doAddChildNode();
            if (addNode == null)
            {
                return;
            }
            this.treeView1.SelectedNode.Nodes.Add(addNode);
            this.treeView1.SelectedNode.Expand();
            this.treeView1.SelectedNode = addNode;
            m_eDocState = eDocState.eDoc_Modified;
        }

        private void removeNodeBtnClick(object sender, EventArgs e)
        {
            TreeNode pnode = this.treeView1.SelectedNode;
            TreeNode parent = pnode.Parent;
            if (parent == null)
            {
                return;
            }

            NodeData parentData = (NodeData)parent.Tag;
            NodeData chidData = (NodeData)pnode.Tag;
            m_vAddSettingPanel[(int)chidData.nodeType].hide();
            parentData.removeChildNode(chidData);
            pnode.Remove();
            this.treeView1.SelectedNode = parent;

            m_eDocState = eDocState.eDoc_Modified;
        }

        private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
        {
            e.Node.BackColor = Color.Yellow;
            if (e.Node.Tag != null )
            {
                NodeData curData = (NodeData)e.Node.Tag;
                m_nCurNodeType = curData.nodeType;
                m_vAddSettingPanel[(int)curData.nodeType].show();
                m_vAddSettingPanel[(int)curData.nodeType].refreshCurNodeData(curData);
                this.infotext.Text = curData.help;
            }
            else
            {
                this.infotext.Text = "no more infogdg";
            }
            this.btnRemoveNode.Enabled = e.Node.Parent != null;
        }

        private void treeView1_BeforeSelect(object sender, TreeViewCancelEventArgs e)
        {
            TreeNode pnode = this.treeView1.SelectedNode;
            if ( pnode != null)
            {
                pnode.BackColor = Color.White;

                if ( pnode.Tag != null)
                {
                    NodeData curData = (NodeData)pnode.Tag;
                    m_vAddSettingPanel[(int)curData.nodeType].hide();
                }
            }
        }

        private void 新建文档ToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            // check if save current document ;
            if ( m_eDocState == eDocState.eDoc_Modified)
            {
                DialogResult dlgret = MessageBox.Show("是否保存当前文档", "notices", MessageBoxButtons.YesNoCancel);
                if (dlgret == DialogResult.Cancel)
                {
                    return;
                }
                else if (dlgret == DialogResult.Yes)
                {
                    this.保存ToolStripMenuItem_Click(sender, e);
                }
                else
                {
                    m_eDocState = eDocState.eDoc_None;
                }
            }

            if (eDocState.eDoc_Modified == m_eDocState)
            {
                MessageBox.Show("cur document do not process can not create new");
                return;
            }

            hideAllNodeSettingPanel();
            this.treeView1.Nodes.Clear();
            for ( int nIdx = 0; nIdx < 4; ++nIdx )
            {
                TreeNode pRound = new TreeNode() ;
                NodeRound tRound = new NodeRound(nIdx);
                pRound.Tag = tRound;
                pRound.Text = tRound.desc;
                this.treeView1.Nodes.Add(pRound);
            }
            m_eDocState = eDocState.eDoc_NewCreated;
            m_strCurDocPath = null;
            this.Text = "untitled";
        }

        private void 保存ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if ( eDocState.eDoc_Modified != m_eDocState)
            {
                MessageBox.Show("当前文档没有修改");
                return;
            }

            if (m_strCurDocPath == null)
            {
                this.另存为ToolStripMenuItem_Click(sender, e);
                return;
            }

            if (saveFile(m_strCurDocPath))
            {
                m_eDocState = eDocState.eDoc_AlreadySaved;
                MessageBox.Show("保存成功");
            }
        }

        private bool saveFile( string strFileName )
        {
            bool bFinish = true;
            foreach (TreeNode node in this.treeView1.Nodes)
            {
                if (node.Tag == null)
                {
                    bFinish = false;
                    break;
                }
                NodeData tData = (NodeData)node.Tag;
                if (tData.checkCanFinish() == false)
                {
                    bFinish = false;
                    break;
                }
            }

            if (bFinish == false)
            {
                if (MessageBox.Show(string.Format("文档没完成:{0},是否继续保存",NodeData.s_errorString), "notices", MessageBoxButtons.YesNo) == DialogResult.No)
                {
                    return false;
                }

            }

            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<root version=\"1.0\"></root>");
            XmlNode root = doc.SelectSingleNode("root");
            foreach ( TreeNode node in this.treeView1.Nodes )
            {
                NodeData tdata = (NodeData)node.Tag ;
                tdata.writeToXmlNode(root,doc);
            }

            string sSaveFileName = strFileName;
            if (!bFinish)
            {
                int nPos = strFileName.LastIndexOf(s_file_ext);
                if (nPos >= 0)
                {
                    sSaveFileName = strFileName.Insert(nPos, s_unfinish_flag);
                }
                else
                {
                    sSaveFileName = strFileName + s_unfinish_flag + s_file_ext;
                }
            }

            doc.Save(sSaveFileName);
            return true ;
        }

        private void 另存为ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (eDocState.eDoc_Modified != m_eDocState && eDocState.eDoc_NewOpen != m_eDocState )
            {
                MessageBox.Show("当前文档没有修改");
                return;
            }

            SaveFileDialog dlg = new SaveFileDialog();
            dlg.AddExtension = true;
            dlg.DefaultExt = s_file_ext ;
            DialogResult ret = dlg.ShowDialog();
            if (DialogResult.OK == ret)
            {
                if ( saveFile(dlg.FileName) )
                {
                    if (m_strCurDocPath == null)
                    {
                        m_strCurDocPath = dlg.FileName;
                        this.Text = m_strCurDocPath;
                    }
                    m_eDocState = eDocState.eDoc_AlreadySaved;
                    MessageBox.Show("保存成功");
                }
            }
        }

        private void equalDivisonClick(object sender, EventArgs e)
        {
            this.pass.Value = 25;
            this.follow.Value = 25;
            this.add.Value = 25;
            this.all.Value = 25;
        }

        private void resetWeightClick(object sender, EventArgs e)
        {
            this.pass.Value = 0;
            this.follow.Value = 0;
            this.add.Value = 0;
            this.all.Value = 0;
        }

        private void strageValueChanged(object sender, EventArgs e)
        {
            decimal totoalWeight = this.pass.Value + this.follow.Value + this.add.Value + all.Value;
            this.leftWeight.Text = string.Format("剩余权重:{0}",100 - totoalWeight );
            Color color = totoalWeight <= 100 ? Color.Green : Color.Red;
            this.leftWeight.ForeColor = color;
        }

        private void 打开文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // check if save current document ;
            if (m_eDocState == eDocState.eDoc_Modified)
            {
                DialogResult dlgret = MessageBox.Show("是否保存当前文档", "notices", MessageBoxButtons.YesNoCancel);
                if (dlgret == DialogResult.Cancel)
                {
                    return;
                }
                else if (dlgret == DialogResult.Yes)
                {
                    this.保存ToolStripMenuItem_Click(sender, e);
                }
                else
                {
                    m_eDocState = eDocState.eDoc_None;
                }
            }

            if (eDocState.eDoc_Modified == m_eDocState)
            {
                MessageBox.Show("cur document do not process can not open new");
                return;
            }

            OpenFileDialog dt = new OpenFileDialog();
            dt.AddExtension = true;
            dt.DefaultExt = s_file_ext ;
            if (dt.ShowDialog() == DialogResult.OK)
            {
                m_strCurDocPath = dt.FileName;
                doLoadFile(m_strCurDocPath);
                m_eDocState = eDocState.eDoc_NewOpen;

                int nPos = m_strCurDocPath.LastIndexOf(s_unfinish_flag);
                if (nPos >= 0)
                {
                    m_strCurDocPath = m_strCurDocPath.Remove(nPos, s_unfinish_flag.Length);
                }
                this.Text = m_strCurDocPath;
            }
        }

        private void doLoadFile(string strFileName)
        {
            XmlDocument doc = new XmlDocument();
            XmlReaderSettings settings = new XmlReaderSettings(); 
            settings.IgnoreComments = true;
            XmlReader reader = XmlReader.Create(strFileName, settings);
            doc.Load(reader);
            XmlNode pRoot = doc.SelectSingleNode("root");
            readXmlNode(pRoot);
            reader.Close();
        }

        private void readXmlNode(XmlNode pRoot)
        {
            if (pRoot.ChildNodes.Count != 4)
            {
                MessageBox.Show("target xml do not cotain 4 round , read failed");
                return;
            }

            hideAllNodeSettingPanel();
            this.treeView1.Nodes.Clear();
            XmlNode pRound = pRoot.FirstChild;
            for (; pRound != null; pRound = pRound.NextSibling)
            {
                Console.WriteLine("node {0}", pRoot.Name);
                NodeRound pdata = new NodeRound(0);
                pdata.readFromXmlElement(pRound);
                // add data to tree view ;
                this.treeView1.Nodes.Add(bindNodeDataToTreeNode(pdata));
            }
        }

        private TreeNode bindNodeDataToTreeNode(NodeData pdata)
        {
            TreeNode pNode = new TreeNode() ;
            pNode.Tag = pdata;
            pNode.Text = pdata.desc;
            foreach ( NodeData tdata in pdata.m_vChildNode.Values)
            {
                TreeNode chiNode = bindNodeDataToTreeNode(tdata);
                pNode.Nodes.Add(chiNode);
            }
            return pNode;
        }

        private void 帮助ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string strTip = " 执行流程：调用者提供，牌型（cardType），手牌对牌型的贡献值张数(contributeCnt)，对最终牌型影响最大的牌面值（faceValue），当前本轮最大下注额是盲注的多少倍（timesBlind）。" ;
            strTip = strTip + "\n\n第一步：找到相应的Round，\n\n第二步：锁定相应的 cardType。 \n\n第三步：寻找所有faceValue配置中，比faceValue参数小的里面，最大的配置，应用之。（如：配置里只配置了，2，8。 那么 [2,8) 都取2，[8,14] 都去14）";
            strTip = strTip + "\n\n第四步：根据timesBlind参数从小到大，从左到右的排序配置，选择自己或者其最左边第一个。第三步同样的意思。 ";
            strTip = strTip + "\n\n备注：如果第三和第四不的选择配置里只有一个，那么默认那一样。如果为空就报错。";
            MessageBox.Show(strTip);
        }

        private void Form11_DragEnter_1(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop)) e.Effect = DragDropEffects.Copy;

        }

        private void Form11_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            string strValidFile = null ;
            foreach (string file in files)
            {
                if ( string.Compare(Path.GetExtension(file),s_file_ext,true) == 0)
                {
                    strValidFile = file;
                    break;
                }
            }

            if (strValidFile == null)
            {
                MessageBox.Show("无法识别的文件格式");
                return;
            }

            // check if save current document ;
            if (m_eDocState == eDocState.eDoc_Modified)
            {
                DialogResult dlgret = MessageBox.Show("是否保存当前文档", "notices", MessageBoxButtons.YesNoCancel);
                if (dlgret == DialogResult.Cancel)
                {
                    return;
                }
                else if (dlgret == DialogResult.Yes)
                {
                    this.保存ToolStripMenuItem_Click(sender, e);
                }
                else
                {
                    m_eDocState = eDocState.eDoc_None;
                }
            }

            if (eDocState.eDoc_Modified == m_eDocState)
            {
                MessageBox.Show("cur document do not process can not open new");
                return;
            }

            // load files 
            m_strCurDocPath = strValidFile;
            doLoadFile(m_strCurDocPath);
            m_eDocState = eDocState.eDoc_NewOpen;

            int nPos = m_strCurDocPath.LastIndexOf(s_unfinish_flag);
            if (nPos >= 0)
            {
                m_strCurDocPath = m_strCurDocPath.Remove(nPos, s_unfinish_flag.Length);
            }
            this.Text = m_strCurDocPath;
        }

    }
}
